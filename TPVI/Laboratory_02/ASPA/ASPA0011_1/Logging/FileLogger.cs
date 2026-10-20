using System.Collections.Concurrent;
using System.Runtime.Serialization;

namespace ASPA0011_1.Logging
{
    public class LogCounter
    {
        private static long _counter = 0;
        public static long Next() => Interlocked.Increment(ref _counter);
    }

    public class CustomFileLogger : ILogger
    {
        private readonly string _categoryName;
        private readonly string _filePath;
        private readonly bool _isDevelopment;
        private static readonly object _lock = new();

        public CustomFileLogger(string categoryName, string filePath, bool isDevelopment)
        {
            _categoryName = categoryName;
            _filePath = filePath;
            _isDevelopment = isDevelopment;
        }
        public IDisposable? BeginScope<TState>(TState state) where TState : notnull => null;
        public bool IsEnabled(LogLevel logLevel)
        {
            if (logLevel == LogLevel.None)
            {
                return false;
            }
            if (_categoryName.StartsWith("Microsoft") || _categoryName.StartsWith("System"))
            {
                return false;
            }
            if ((logLevel == LogLevel.Trace || logLevel == LogLevel.Debug) && !_isDevelopment)
            {
                return false;
            }
            return true;
        }

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception, Func <TState, Exception?, string> formatter)
        {
            if(!IsEnabled(logLevel))
            {
                return;
            }

            long num = LogCounter.Next();
            string record = $"[{num}] [{DateTime.Now: yyyy-MM-dd HH:mm:ss.fff}] [{logLevel}] [{_categoryName}] {formatter(state, exception)}";
            if (exception != null)
            {
                record += Environment.NewLine + exception;
            }

            lock (_lock)
            {
                File.AppendAllText(_filePath, record + Environment.NewLine);
            }
        }

        public class CustomFileLoggerProvider : ILoggerProvider
        {
            private readonly string _filePath;
            private readonly bool _isDevelopment;
            private readonly ConcurrentDictionary<string, CustomFileLogger> _loggers = new();

            public CustomFileLoggerProvider(string filePath, bool isDevelopment)
            {
                _filePath = filePath;
                _isDevelopment = isDevelopment;
            }

            public ILogger CreateLogger(string categoryName) =>
                _loggers.GetOrAdd(categoryName, name => new CustomFileLogger(name, _filePath, _isDevelopment));

            public void Dispose() => _loggers.Clear();
        }
    }
}