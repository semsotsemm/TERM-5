using ASPA0011_1.Logging;
using ASPA0011_1.Services;
using System.Text;
using static ASPA0011_1.Logging.CustomFileLogger;

var builder = WebApplication.CreateBuilder(args);

builder.Logging.ClearProviders();

builder.Logging.AddSimpleConsole(options =>
{
    options.TimestampFormat = "[yyyy-MM-dd HH:mm:ss] ";
});
builder.Logging.AddFilter<Microsoft.Extensions.Logging.Console.ConsoleLoggerProvider>((category, level) =>
{
    if (category != null && (category.StartsWith("Microsoft") || category.StartsWith("System")))
        return level >= LogLevel.Error; 

    return level >= LogLevel.Warning; 
});

if (builder.Environment.IsDevelopment())
{
    builder.Logging.AddDebug();
    builder.Logging.AddFilter<Microsoft.Extensions.Logging.Debug.DebugLoggerProvider>((category, level) =>
    {
        if (category != null && (category.StartsWith("Microsoft") || category.StartsWith("System")))
            return false;
        return level == LogLevel.Debug;
    });
}

string logFilePath = builder.Configuration["FileLogPath"] ?? "logs/aspa0011_log.txt";
builder.Logging.AddProvider(new CustomFileLoggerProvider(logFilePath, builder.Environment.IsDevelopment()));

builder.Services.AddControllers();
builder.Services.AddSingleton<ChannelManagerService>();

var app = builder.Build();
var logger = app.Services.GetRequiredService<ILoggerFactory>().CreateLogger("ASPA0011_1.Startup");

try
{
    if (!File.Exists("appsettings.json"))
    {
        logger.LogCritical("Критическая ошибка: отсутствует конфигурационный файл appsettings.json!");
        throw new FileNotFoundException("appsettings.json not found");
    }

    var dir = Path.GetDirectoryName(logFilePath);
    if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
        Directory.CreateDirectory(dir);

    using var fs = File.Open(logFilePath, FileMode.OpenOrCreate, FileAccess.Write, FileShare.ReadWrite);
}
catch (Exception ex)
{
    logger.LogCritical(ex, "Критическая ошибка при инициализации приложения (не открывается файл журнала или конфигурации).");
    throw;
}

app.Lifetime.ApplicationStarted.Register(() =>
    logger.LogInformation("Приложение ASPA0011_1 успешно запущено."));

app.Lifetime.ApplicationStopping.Register(() =>
    logger.LogInformation("Приложение ASPA0011_1 останавливается."));

if (app.Environment.IsDevelopment())
{
    app.Use(async (context, next) =>
    {
        context.Request.EnableBuffering();
        string reqBody = string.Empty;
        if (context.Request.ContentLength > 0)
        {
            using var reader = new StreamReader(context.Request.Body, Encoding.UTF8, leaveOpen: true);
            reqBody = await reader.ReadToEndAsync();
            context.Request.Body.Position = 0;
        }

        logger.LogTrace("Входящий запрос: {Method} {Path}", context.Request.Method, context.Request.Path);

        var originalBodyStream = context.Response.Body;
        using var responseBody = new MemoryStream();
        context.Response.Body = responseBody;

        await next();

        context.Response.Body.Seek(0, SeekOrigin.Begin);
        string respText = await new StreamReader(context.Response.Body).ReadToEndAsync();
        context.Response.Body.Seek(0, SeekOrigin.Begin);

        logger.LogTrace("Исходящий ответ: Status={StatusCode}", context.Response.StatusCode);
        logger.LogDebug("Полная информация. Запрос: {Method} {Path} Headers={Headers} Body={ReqBody} | Ответ: Status={Status} Body={RespBody}",
            context.Request.Method, context.Request.Path, context.Request.Headers.ToString(), reqBody, context.Response.StatusCode, respText);

        await responseBody.CopyToAsync(originalBodyStream);
    });
}

app.MapControllers();
app.Run();