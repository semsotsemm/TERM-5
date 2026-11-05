using System;
using System.IO;
using System.Linq;
using UnityEditor;
using UnityEditor.SceneManagement;
using UnityEngine;

[InitializeOnLoad]
public static class TankLabTools
{
    static readonly string command_path = Path.GetFullPath("Logs/tank-lab-command.txt");
    static double next_check;

    static TankLabTools()
    {
        EditorApplication.update += Poll;
    }

    static void Poll()
    {
        if (EditorApplication.timeSinceStartup < next_check) return;
        next_check = EditorApplication.timeSinceStartup + 0.5;
        if (EditorApplication.isCompiling || EditorApplication.isUpdating || !File.Exists(command_path)) return;
        if (EditorApplication.isPlayingOrWillChangePlaymode)
        {
            EditorApplication.isPlaying = false;
            return;
        }
        string command = File.ReadAllText(command_path).Trim();
        File.Delete(command_path);
        try
        {
            if (command == "audit") Audit();
            if (command == "setup") Setup();
            if (command == "test") TankLabChecks.Run();
            if (command == "preview") TankLabChecks.Capture("Logs/tank-lab-preview.png");
            if (command == "visuals") TankLabChecks.Visuals();
        }
        catch (Exception error)
        {
            File.WriteAllText("Logs/tank-lab-error.txt", error.ToString());
            Debug.LogException(error);
        }
    }

    [Serializable] public class ObjectInfo
    {
        public string path;
        public string id;
        public Vector3 position, local_position, angles, local_angles, scale, right, up, forward;
        public Bounds bounds;
        public string[] components;
    }
    [Serializable] public class SceneInfo
    {
        public string scene;
        public bool dirty;
        public string tank, tower, barrel, pivot;
        public ObjectInfo[] objects;
    }

    static string ObjectPath(Transform item)
    {
        return item == null ? "" : item.parent == null ? item.name : ObjectPath(item.parent) + "/" + item.name;
    }

    static void Audit()
    {
        var scene = EditorSceneManager.GetActiveScene();
        var tank = UnityEngine.Object.FindAnyObjectByType<tank_script>();
        var report = new SceneInfo { scene = scene.path, dirty = scene.isDirty };
        if (tank != null)
        {
            report.tank = ObjectPath(tank.transform);
            report.tower = ObjectPath(tank.tank_tower);
            report.barrel = ObjectPath(tank.tank_barrel);
            report.pivot = ObjectPath(tank.camera_pivot);
        }
        report.objects = scene.GetRootGameObjects().SelectMany(root => root.GetComponentsInChildren<Transform>(true)).Select(item =>
        {
            var renderer = item.GetComponent<Renderer>();
            return new ObjectInfo
            {
                path = ObjectPath(item), id = GlobalObjectId.GetGlobalObjectIdSlow(item).ToString(),
                position = item.position, local_position = item.localPosition,
                angles = item.eulerAngles, local_angles = item.localEulerAngles, scale = item.lossyScale,
                right = item.right, up = item.up, forward = item.forward,
                bounds = renderer != null ? renderer.bounds : new Bounds(item.position, Vector3.zero),
                components = item.GetComponents<Component>().Where(c => c != null).Select(c => c.GetType().Name).ToArray()
            };
        }).ToArray();
        File.WriteAllText("Logs/tank-lab-audit.json", JsonUtility.ToJson(report, true));
        Debug.Log("Tank lab: scene audit saved.");
    }

    [MenuItem("Tank Lab/Настроить задания")]
    public static void Setup()
    {
        var scene = EditorSceneManager.GetActiveScene();
        if (scene.path != "Assets/tank_scene.unity") throw new InvalidOperationException("Open tank_scene first.");
        var old_controller = UnityEngine.Object.FindAnyObjectByType<tank_script>();
        if (old_controller == null || old_controller.tank_barrel == null || old_controller.tank_tower == null)
            throw new InvalidOperationException("Tank, tower and barrel references are required.");
        if (GameObject.Find("Tank Lab Objects") != null) throw new InvalidOperationException("Scene is already configured.");

        string backup = AssetDatabase.GenerateUniqueAssetPath("Assets/_Recovery/tank_scene_before_lab_tasks.unity");
        if (!EditorSceneManager.SaveScene(scene, backup, true)) throw new IOException("Could not back up the live scene.");
        Undo.IncrementCurrentGroup();
        Undo.SetCurrentGroupName("Complete tank laboratory");
        Transform model = old_controller.transform;
        Transform tower = old_controller.tank_tower;
        Transform gun = old_controller.tank_barrel;
        Transform camera_pivot = old_controller.camera_pivot;
        Camera camera = camera_pivot.GetComponentInChildren<Camera>();
        Vector3 forward = Vector3.ProjectOnPlane(model.up, Vector3.up).normalized;
        if (forward.sqrMagnitude < 0.5f) forward = Vector3.back;
        Bounds hull_bounds = CombinedBounds(model.GetComponentsInChildren<Renderer>().Where(r => !r.transform.IsChildOf(tower)));
        Bounds gun_bounds = CombinedBounds(gun.GetComponentsInChildren<Renderer>());

        var root = new GameObject("Tank");
        Undo.RegisterCreatedObjectUndo(root, "Tank root");
        root.transform.position = model.position;
        root.transform.rotation = Quaternion.LookRotation(forward, Vector3.up);
        Undo.SetTransformParent(model, root.transform, "Normalize imported model axes");
        var old_body = model.GetComponent<Rigidbody>();
        if (old_body != null) Undo.DestroyObjectImmediate(old_body);
        Undo.DestroyObjectImmediate(old_controller);
        Rigidbody body = Undo.AddComponent<Rigidbody>(root);
        body.mass = 100f;
        body.useGravity = true;
        body.constraints = RigidbodyConstraints.FreezeRotationX | RigidbodyConstraints.FreezeRotationZ;
        body.interpolation = RigidbodyInterpolation.Interpolate;
        body.collisionDetectionMode = CollisionDetectionMode.ContinuousDynamic;
        BoxCollider tank_collider = Undo.AddComponent<BoxCollider>(root);
        tank_collider.center = root.transform.InverseTransformPoint(hull_bounds.center);
        tank_collider.size = hull_bounds.size;
        tank_script controller = Undo.AddComponent<tank_script>(root);
        controller.tank_tower = tower;

        // Новый шарнир у казённой части сохраняет мировые координаты всей геометрии дула.
        Transform barrel_pivot = new GameObject("BarrelPivot").transform;
        Undo.RegisterCreatedObjectUndo(barrel_pivot.gameObject, "Barrel hinge");
        barrel_pivot.position = new Vector3(gun_bounds.center.x, gun.position.y, gun_bounds.max.z);
        barrel_pivot.rotation = Quaternion.LookRotation(forward, Vector3.up);
        barrel_pivot.SetParent(tower, true);
        Undo.SetTransformParent(gun, barrel_pivot, "Move gun under hinge");
        controller.tank_barrel = barrel_pivot;

        Undo.SetTransformParent(camera_pivot, root.transform, "Separate camera from imported axes");
        camera_pivot.name = "CameraPivot";
        camera_pivot.localScale = Vector3.one;
        camera_pivot.position = root.transform.position + Vector3.up * controller.camera_height;
        camera_pivot.rotation = Quaternion.Euler(controller.start_pitch, Mathf.Atan2(forward.x, forward.z) * Mathf.Rad2Deg, 0f);
        camera.transform.localPosition = new Vector3(0f, 0f, -24f);
        camera.transform.localRotation = Quaternion.identity;
        camera.transform.localScale = Vector3.one;
        camera.nearClipPlane = 0.1f;
        camera.farClipPlane = 1200f;
        controller.camera_pivot = camera_pivot;

        GameObject extras = new GameObject("Tank Lab Objects");
        Undo.RegisterCreatedObjectUndo(extras, "Lab objects");
        Material red = MaterialAsset("bomb_red", new Color(0.85f, 0.035f, 0.02f), false);
        Material red_line = MaterialAsset("red_zone", new Color(1f, 0.05f, 0.03f), true);
        Material blue_line = MaterialAsset("blue_zone", new Color(0.05f, 0.25f, 1f), true);
        Material bomb_line = MaterialAsset("bomb_zone", new Color(1f, 0.55f, 0.02f), true);

        GameObject wall = GameObject.Find("Cube");
        if (wall == null) throw new InvalidOperationException("Wall Cube was not found.");
        Undo.RecordObject(wall, "Name wall");
        wall.name = "Rotating Wall";
        Rigidbody wall_body = wall.GetComponent<Rigidbody>();
        Undo.RecordObject(wall_body, "Anchor rotating wall");
        wall_body.isKinematic = true;
        wall_body.useGravity = false;
        GameObject wall_zone = new GameObject("Wall Trigger");
        wall_zone.transform.SetParent(extras.transform);
        wall_zone.transform.position = wall.transform.position + Vector3.forward * 12f;
        wall_zone.transform.position = new Vector3(wall_zone.transform.position.x, 6f, wall_zone.transform.position.z);
        BoxCollider wall_box = wall_zone.AddComponent<BoxCollider>();
        wall_box.isTrigger = true;
        wall_box.size = new Vector3(46f, 12f, 18f);
        wall_zone.AddComponent<wall_trigger>().wall = wall_body;
        Outline(wall_zone.transform, wall_box.size, bomb_line);

        CreateLightZone(extras.transform, "Red", new Vector3(-65f, 6f, -30f), Color.red, red_line);
        CreateLightZone(extras.transform, "Blue", new Vector3(65f, 6f, -30f), Color.blue, blue_line);
        AudioClip background = AssetDatabase.LoadAssetAtPath<AudioClip>("Assets/Audio/background_wind.wav");
        AudioClip engine = AssetDatabase.LoadAssetAtPath<AudioClip>("Assets/Audio/tank_engine.wav");
        AudioClip impact = AssetDatabase.LoadAssetAtPath<AudioClip>("Assets/Audio/bomb_impact.wav");
        if (background == null || engine == null || impact == null) throw new InvalidOperationException("Audio files were not imported.");
        AudioSource ambience = extras.AddComponent<AudioSource>();
        ambience.clip = background;
        ambience.loop = true;
        ambience.playOnAwake = true;
        ambience.spatialBlend = 0f;
        ambience.volume = 0.35f;
        AudioSource motor = root.AddComponent<AudioSource>();
        motor.clip = engine;
        motor.loop = true;
        motor.playOnAwake = false;
        motor.volume = 0f;
        motor.spatialBlend = 0.25f;
        motor.minDistance = 10f;
        motor.maxDistance = 100f;
        controller.engine_audio = motor;

        EnsureFolder("Assets/Prefabs");
        GameObject bomb = GameObject.CreatePrimitive(PrimitiveType.Capsule);
        bomb.name = "Bomb";
        bomb.transform.localScale = new Vector3(0.8f, 1.3f, 0.8f);
        bomb.GetComponent<Renderer>().sharedMaterial = red;
        Rigidbody bomb_body = bomb.AddComponent<Rigidbody>();
        bomb_body.mass = 3f;
        bomb_body.collisionDetectionMode = CollisionDetectionMode.ContinuousDynamic;
        bomb.AddComponent<bomb_script>().impact_sound = impact;
        GameObject prefab = PrefabUtility.SaveAsPrefabAsset(bomb, "Assets/Prefabs/Bomb.prefab");
        UnityEngine.Object.DestroyImmediate(bomb);
        bomb_dropper dropper = Undo.AddComponent<bomb_dropper>(root);
        dropper.bomb_prefab = prefab;
        dropper.ground = GameObject.Find("Plane").GetComponent<Collider>();
        dropper.zone_material = bomb_line;
        dropper.tank_size = new Vector2(hull_bounds.size.x, hull_bounds.size.z);

        // Сцена танка должна запускаться и из сборки, а не только из открытой вкладки редактора.
        EditorBuildSettings.scenes = new[] { new EditorBuildSettingsScene(scene.path, true) };
        EditorSceneManager.MarkSceneDirty(scene);
        AssetDatabase.SaveAssets();
        if (!EditorSceneManager.SaveScene(scene)) throw new IOException("Could not save completed scene.");
        File.WriteAllText("Logs/tank-lab-setup.txt", "Setup complete. Backup: " + backup + "\nHull: " + hull_bounds + "\nGun: " + gun_bounds);
        Selection.activeGameObject = root;
        Audit();
        Debug.Log("Tank lab: all tasks configured and saved.");
    }

    static Bounds CombinedBounds(System.Collections.Generic.IEnumerable<Renderer> renderers)
    {
        Renderer[] items = renderers.ToArray();
        if (items.Length == 0) throw new InvalidOperationException("Model has no renderers.");
        Bounds bounds = items[0].bounds;
        foreach (Renderer renderer in items.Skip(1)) bounds.Encapsulate(renderer.bounds);
        return bounds;
    }

    static void EnsureFolder(string path)
    {
        if (!AssetDatabase.IsValidFolder(path))
            AssetDatabase.CreateFolder(Path.GetDirectoryName(path).Replace('\\', '/'), Path.GetFileName(path));
    }

    static Material MaterialAsset(string name, Color color, bool unlit)
    {
        EnsureFolder("Assets/Materials");
        string path = "Assets/Materials/" + name + ".mat";
        Material material = AssetDatabase.LoadAssetAtPath<Material>(path);
        if (material == null)
        {
            material = new Material(Shader.Find(unlit ? "Universal Render Pipeline/Unlit" : "Universal Render Pipeline/Lit"));
            AssetDatabase.CreateAsset(material, path);
        }
        material.SetColor("_BaseColor", color);
        return material;
    }

    static void CreateLightZone(Transform parent, string name, Vector3 position, Color color, Material line_material)
    {
        GameObject zone = new GameObject(name + " Light Trigger");
        zone.transform.SetParent(parent);
        zone.transform.position = position;
        BoxCollider box = zone.AddComponent<BoxCollider>();
        box.isTrigger = true;
        box.size = new Vector3(14f, 12f, 120f);
        GameObject light_object = new GameObject(name + " Point Light");
        light_object.transform.SetParent(parent);
        light_object.transform.position = new Vector3(position.x, 12f, position.z);
        Light light = light_object.AddComponent<Light>();
        light.type = LightType.Point;
        light.color = color;
        light.range = 100f;
        light.intensity = 0f;
        light.renderMode = LightRenderMode.ForcePixel;
        zone.AddComponent<light_trigger>().point_light = light;
        Outline(zone.transform, box.size, line_material);
    }

    static void Outline(Transform parent, Vector3 size, Material material)
    {
        GameObject marker = new GameObject("Zone Outline");
        marker.transform.SetParent(parent, false);
        LineRenderer line = marker.AddComponent<LineRenderer>();
        line.sharedMaterial = material;
        line.useWorldSpace = false;
        line.loop = true;
        line.widthMultiplier = 0.25f;
        line.positionCount = 4;
        float y = -parent.position.y + 0.1f;
        line.SetPositions(new[] { new Vector3(-size.x/2f,y,-size.z/2f), new Vector3(size.x/2f,y,-size.z/2f),
            new Vector3(size.x/2f,y,size.z/2f), new Vector3(-size.x/2f,y,size.z/2f) });
    }
}
