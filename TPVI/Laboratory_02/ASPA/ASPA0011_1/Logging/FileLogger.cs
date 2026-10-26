using Microsoft.Extensions.Logging;

namespace ASPA0011_1.Logging
{
    public class FileLogger : ILogger
    {
        private readonly string _filePath;
        private static readonly object _lock = new object();
        private static int _counter = 1;

        public FileLogger(string path)
        {
            _filePath = path;
        }

        public IDisposable? BeginScope<TState>(TState state) => null;

        public bool IsEnabled(LogLevel logLevel) => true; // Фильтрация задается в appsettings

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception, Func<TState, Exception?, string> formatter)
        {
            if (formatter != null)
            {
                lock (_lock)
                {
                    string message = formatter(state, exception);
                    string logRecord = $"[{_counter++}] {DateTime.Now:dd.MM.yyyy HH:mm:ss} [{logLevel}] {message}";

                    if (exception != null)
                    {
                        logRecord += Environment.NewLine + exception.ToString();
                    }

                    File.AppendAllText(_filePath, logRecord + Environment.NewLine);
                }
            }
        }
    }

    public class FileLoggerProvider : ILoggerProvider
    {
        private readonly string _path;
        public FileLoggerProvider(string path) => _path = path;
        public ILogger CreateLogger(string categoryName) => new FileLogger(_path);
        public void Dispose() { }
    }

    public static class FileLoggerExtensions
    {
        public static ILoggingBuilder AddFile(this ILoggingBuilder builder, string filePath)
        {
            builder.AddProvider(new FileLoggerProvider(filePath));
            return builder;
        }
    }
}