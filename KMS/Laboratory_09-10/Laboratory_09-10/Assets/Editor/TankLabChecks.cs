using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;

// Проверка лабораторной через настоящие физические события Play Mode.
[InitializeOnLoad]
public static class TankLabChecks
{
    const BindingFlags private_instance = BindingFlags.Instance | BindingFlags.NonPublic;
    static int stage = -1;
    static double next;
    static tank_script tank;
    static Rigidbody body;
    static bomb_dropper dropper;
    static Vector3 original_position;
    static Quaternion original_rotation, wall_rotation;
    static Rigidbody wall;
    static GameObject probe_bomb;
    static bool old_play_options;
    static List<string> checks = new List<string>();
    static List<string> errors = new List<string>();

    [Serializable] class Report
    {
        public string[] passed;
        public string[] errors;
    }

    static TankLabChecks()
    {
        EditorApplication.update += Tick;
    }

    [MenuItem("Tank Lab/Проверить задания")]
    public static void Run()
    {
        if (EditorApplication.isPlaying || stage >= 0) throw new InvalidOperationException("Stop Play Mode before testing.");
        if (EditorSceneManager.GetActiveScene().path != "Assets/tank_scene.unity") throw new InvalidOperationException("Open tank_scene first.");
        if (!EditorSceneManager.SaveScene(EditorSceneManager.GetActiveScene())) throw new IOException("Could not save scene.");
        checks.Clear();
        errors.Clear();
        old_play_options = EditorSettings.enterPlayModeOptionsEnabled;
        // The test state remains in this editor assembly while Play Mode is running.
        EditorSettings.enterPlayModeOptionsEnabled = true;
        SessionState.SetInt("TankLabOldOptions", (int)EditorSettings.enterPlayModeOptions);
        EditorSettings.enterPlayModeOptions = EnterPlayModeOptions.DisableDomainReload | EnterPlayModeOptions.DisableSceneReload;
        stage = 0;
        Application.logMessageReceived += OnLog;
        EditorApplication.isPlaying = true;
    }

    static void OnLog(string text, string trace, LogType type)
    {
        if (type == LogType.Error || type == LogType.Exception || type == LogType.Assert) errors.Add(text);
    }

    static void Check(bool condition, string name)
    {
        if (!condition) throw new InvalidOperationException("FAILED: " + name);
        checks.Add(name);
    }

    static void SetField(string name, object value)
    {
        typeof(tank_script).GetField(name, private_instance).SetValue(tank, value);
    }

    static void Call(string name, params object[] values)
    {
        typeof(tank_script).GetMethod(name, private_instance).Invoke(tank, values);
    }

    static void Teleport(Vector3 position)
    {
        body.position = position;
        tank.transform.position = position;
        Physics.SyncTransforms();
    }

    static void Advance(int next_stage, double delay)
    {
        stage = next_stage;
        next = EditorApplication.timeSinceStartup + delay;
    }

    static void Tick()
    {
        if (stage < 0 || EditorApplication.isCompiling || EditorApplication.timeSinceStartup < next) return;
        try
        {
            if (stage == 99)
            {
                if (EditorApplication.isPlayingOrWillChangePlaymode) return;
                EditorSettings.enterPlayModeOptionsEnabled = old_play_options;
                EditorSettings.enterPlayModeOptions = (EnterPlayModeOptions)SessionState.GetInt("TankLabOldOptions", 0);
                EditorSceneManager.OpenScene("Assets/tank_scene.unity");
                stage = -1;
                File.WriteAllText("Logs/tank-lab-tests.json", JsonUtility.ToJson(new Report { passed = checks.ToArray(), errors = errors.ToArray() }, true));
                Capture("Logs/tank-lab-preview.png");
                Debug.Log("Tank lab checks complete: " + checks.Count + " passed, " + errors.Count + " errors.");
                return;
            }
            if (!EditorApplication.isPlaying) return;
            switch (stage)
            {
                case 0:
                    Advance(1, 0.7);
                    break;
                case 1:
                    tank = UnityEngine.Object.FindAnyObjectByType<tank_script>();
                    body = tank.GetComponent<Rigidbody>();
                    original_position = body.position;
                    original_rotation = body.rotation;
                    tank.enabled = false;
                    dropper = tank.GetComponent<bomb_dropper>();
                    dropper.enabled = false;
                    body.isKinematic = true;
                    foreach (float yaw in new[] { 0f, 45f, 179f, 270f, 359f })
                    foreach (float pitch in new[] { -35f, 0f, 65f })
                    {
                        SetField("camera_yaw", yaw);
                        SetField("target_cam_pitch", pitch);
                        Call("UpdateView", 20f, false);
                        Vector3 expected = Quaternion.Euler(0f, yaw, 0f) * Vector3.forward;
                        Vector3 actual = Vector3.ProjectOnPlane(tank.tank_barrel.forward, Vector3.up).normalized;
                        Check(Vector3.Dot(expected, actual) > 0.999f, "Turret yaw " + yaw + ", camera pitch " + pitch);
                        float elevation = Mathf.Asin(tank.tank_barrel.forward.normalized.y) * Mathf.Rad2Deg;
                        Check(Mathf.Abs(elevation - Mathf.Clamp(-pitch, -6f, 20f)) < 0.02f, "Barrel elevation limited: " + elevation);
                    }
                    Quaternion saved_tower = tank.tank_tower.rotation;
                    Quaternion saved_barrel = tank.tank_barrel.localRotation;
                    body.rotation *= Quaternion.Euler(0f, 90f, 0f);
                    SetField("camera_yaw", 150f);
                    SetField("target_cam_pitch", -30f);
                    Call("UpdateView", 1f, true);
                    Check(Quaternion.Angle(saved_tower, tank.tank_tower.rotation) < 0.02f && Quaternion.Angle(saved_barrel, tank.tank_barrel.localRotation) < 0.02f, "Free look holds gun orientation while hull turns");
                    SetField("move_input", 1f);
                    Call("UpdateEngineSound", 1f);
                    Check(tank.engine_audio.isPlaying && tank.engine_audio.loop, "Looping engine sound starts with movement");
                    SetField("move_input", 0f);
                    Call("UpdateEngineSound", 2f);
                    Check(!tank.engine_audio.isPlaying, "Engine sound stops at idle");
                    AudioSource ambience = GameObject.Find("Tank Lab Objects").GetComponent<AudioSource>();
                    Check(ambience.isPlaying && ambience.loop, "Background ambience loops");
                    body.rotation = original_rotation;
                    Teleport(new Vector3(-65f, original_position.y, 0f));
                    Advance(2, 0.3);
                    break;
                case 2:
                    Check(GameObject.Find("Red Point Light").GetComponent<Light>().intensity == 10f, "Red OnTriggerEnter sets intensity 10");
                    Check(GameObject.Find("Blue Point Light").GetComponent<Light>().intensity == 0f, "Blue remains off outside its trigger");
                    SetField("camera_yaw", 180f);
                    SetField("target_cam_pitch", 18f);
                    Call("UpdateView", 20f, false);
                    Capture("Logs/tank-lab-red-preview.png");
                    Teleport(original_position);
                    Advance(3, 0.3);
                    break;
                case 3:
                    Check(GameObject.Find("Red Point Light").GetComponent<Light>().intensity == 0f, "Red OnTriggerExit sets intensity 0");
                    Teleport(new Vector3(65f, original_position.y, 0f));
                    Advance(4, 0.3);
                    break;
                case 4:
                    Check(GameObject.Find("Blue Point Light").GetComponent<Light>().intensity == 10f, "Blue OnTriggerEnter sets intensity 10");
                    Teleport(original_position);
                    Advance(5, 0.3);
                    break;
                case 5:
                    Check(GameObject.Find("Blue Point Light").GetComponent<Light>().intensity == 0f, "Blue OnTriggerExit sets intensity 0");
                    wall = GameObject.Find("Rotating Wall").GetComponent<Rigidbody>();
                    wall_rotation = wall.rotation;
                    Vector3 zone = GameObject.Find("Wall Trigger").transform.position;
                    Teleport(new Vector3(zone.x, original_position.y, zone.z));
                    Advance(6, 0.6);
                    break;
                case 6:
                    Check(Quaternion.Angle(wall_rotation, wall.rotation) > 5f, "OnTriggerStay rotates wall while tank is inside");
                    Teleport(original_position);
                    Advance(7, 0.3);
                    break;
                case 7:
                    wall_rotation = wall.rotation;
                    Advance(8, 0.3);
                    break;
                case 8:
                    Check(Quaternion.Angle(wall_rotation, wall.rotation) < 0.1f, "Wall stops rotating after tank leaves trigger");
                    dropper.drop_interval = 0.01f;
                    dropper.DropBombs();
                    Advance(9, 0.6);
                    break;
                case 9:
                    var bombs = UnityEngine.Object.FindObjectsByType<bomb_script>(FindObjectsSortMode.None);
                    Check(bombs.Length == dropper.bomb_count, "Bomb prefab instantiated " + dropper.bomb_count + " times");
                    Vector3 center = original_position + tank.transform.forward * (dropper.tank_size.y * 1.5f);
                    foreach (var bomb in bombs)
                    {
                        Vector3 offset = bomb.transform.position - center;
                        Check(Mathf.Abs(Vector3.Dot(offset, tank.transform.right)) <= dropper.tank_size.x + 0.1f && Mathf.Abs(Vector3.Dot(offset, tank.transform.forward)) <= dropper.tank_size.y + 0.1f,
                            "Random bomb remains within double-size area");
                    }
                    probe_bomb = UnityEngine.Object.Instantiate(dropper.bomb_prefab, new Vector3(150f, 3f, 150f), Quaternion.identity);
                    probe_bomb.GetComponent<bomb_script>().ground = dropper.ground;
                    Teleport(new Vector3(100f, original_position.y, 0f));
                    Advance(10, 0.9);
                    break;
                case 10:
                    Check(probe_bomb == null, "Capsule bomb disappears on real ground collision");
                    Check(UnityEngine.Object.FindObjectsByType<AudioSource>(FindObjectsSortMode.None).Any(a => a.name == "Bomb impact sound" && a.isPlaying), "Ground collision plays one-shot impact sound");
                    Advance(11, 5.0);
                    break;
                case 11:
                    Check(UnityEngine.Object.FindObjectsByType<bomb_script>(FindObjectsSortMode.None).Length == 0, "All dropped bombs cleaned up after landing");
                    body.isKinematic = false;
                    body.rotation = original_rotation;
                    Teleport(original_position);
                    SetField("move_input", 1f);
                    SetField("turn_input", 0f);
                    Call("FixedUpdate");
                    Advance(12, 0.6);
                    break;
                case 12:
                    Check(Vector3.Dot(body.position - original_position, tank.transform.forward) > 2f, "Tank physically moves forward on the plane");
                    Check(body.position.y > 0f, "Tank remains above ground");
                    GameObject.Find("Wall Trigger").GetComponent<Collider>().enabled = false;
                    wall.rotation = Quaternion.identity;
                    body.rotation = original_rotation;
                    Teleport(new Vector3(wall.position.x, original_position.y, wall.position.z + 10f));
                    SetField("move_input", 1f);
                    Call("FixedUpdate");
                    Advance(13, 1.0);
                    break;
                case 13:
                    Check(tank.GetComponent<Collider>().bounds.min.z >= wall.GetComponent<Collider>().bounds.max.z - 0.3f, "Wall collision blocks the moving tank");
                    Finish();
                    break;
            }
        }
        catch (Exception error)
        {
            errors.Add(error.ToString());
            Finish();
        }
    }

    static void Finish()
    {
        Application.logMessageReceived -= OnLog;
        stage = 99;
        EditorApplication.isPlaying = false;
        next = EditorApplication.timeSinceStartup + 0.3;
    }

    public static void Visuals()
    {
        tank_script controller = UnityEngine.Object.FindAnyObjectByType<tank_script>();
        Vector3 start = controller.transform.position;
        foreach (string color in new[] { "Red", "Blue" })
        {
            float x = color == "Red" ? -65f : 65f;
            controller.transform.position = new Vector3(x, start.y, 0f);
            controller.camera_pivot.position = controller.transform.position + Vector3.up * controller.camera_height;
            controller.camera_pivot.rotation = Quaternion.Euler(18f, 180f, 0f);
            Light light = GameObject.Find(color + " Point Light").GetComponent<Light>();
            light.transform.position = controller.transform.position + Vector3.up * 5f;
            light.intensity = 10f;
            Capture("Logs/tank-lab-" + color.ToLowerInvariant() + "-visual.png");
            light.intensity = 0f;
        }
        EditorSceneManager.OpenScene("Assets/tank_scene.unity");
    }

    public static void Capture(string path)
    {
        Camera camera = Camera.main;
        if (camera == null) return;
        RenderTexture previous = camera.targetTexture;
        RenderTexture active = RenderTexture.active;
        RenderTexture target = new RenderTexture(1024, 600, 24);
        Texture2D image = new Texture2D(1024, 600, TextureFormat.RGB24, false);
        try
        {
            camera.targetTexture = target;
            camera.Render();
            RenderTexture.active = target;
            image.ReadPixels(new Rect(0f, 0f, 1024f, 600f), 0, 0);
            image.Apply();
            File.WriteAllBytes(path, image.EncodeToPNG());
        }
        finally
        {
            camera.targetTexture = previous;
            RenderTexture.active = active;
            target.Release();
            UnityEngine.Object.DestroyImmediate(target);
            UnityEngine.Object.DestroyImmediate(image);
        }
    }
}
