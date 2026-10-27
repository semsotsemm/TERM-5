using System.Collections.Concurrent;
using ASPA0011_1.Models;
using Microsoft.Extensions.Configuration; 
using Microsoft.Extensions.Logging;


namespace ASPA0011_1.Services
{
    public class ChannelManager
    {
        private readonly Dictionary<string, Func<ChannelCommand, object>> _handlers;
        private readonly ConcurrentDictionary<string, Channel> _Channels = new();
        private readonly int _MaxChannels = 1000;

        private readonly ILogger<ChannelManager> _logger;
        private readonly int _waitEnqueueTimeout;

        public ChannelManager(ILogger<ChannelManager> logger, IConfiguration config)
        {
            _logger = logger;
            _waitEnqueueTimeout = config.GetValue<int>("WaitEnqueue", 5);

            _handlers = new Dictionary<string, Func<ChannelCommand, object>>
            {
                { "new", CreateChannel},
                { "close", CloseChannel },
                { "open", OpenChannel },
                { "del", DelChannel },
                { "enqueue", EnqueueChannel },
                { "dequeue", DequeueChannel },
                { "peek", PeekChannel }
            };
        }

        public object ProcessCommand(ChannelCommand command_dto)
        {
            if (string.IsNullOrWhiteSpace(command_dto.Command))
            {
                throw new ArgumentException("Ошибка 1: Команда не указана.");
            }
            if (_handlers.TryGetValue(command_dto.Command, out var handlerMethod))
            {
                return handlerMethod.Invoke(command_dto);
            }
            throw new ArgumentException("Ошибка 2: Переданная команда не найдена в списке доступных.");
        }

        public Channel CreateChannel(ChannelCommand channel_command)
        {
            if (_Channels.Count >= _MaxChannels)
            {
                throw new InvalidOperationException("Ошибка 4: Превышен глобальный лимит активных каналов.");
            }
            if (string.IsNullOrWhiteSpace(channel_command.Name))
            {
                throw new ArgumentException("Ошибка 3: Имя канала (name) не может быть пустым.");
            }
            if (string.IsNullOrWhiteSpace(channel_command.Description))
            {
                throw new ArgumentException("Ошибка 3: Описание канала (description) не может быть пустым.");
            }
            string newChannelGuid = Guid.NewGuid().ToString();
            string newChannelName = channel_command.Name;
            string newChannelDescription = channel_command.Description;

            Channel newChannel = new Channel
            {
                Id = newChannelGuid,
                Name = newChannelName,
                State = "ACTIVE",
                Description = newChannelDescription,
                Queue = System.Threading.Channels.Channel.CreateUnbounded<string>()
            };
            _Channels.TryAdd(newChannelGuid, newChannel);

            _logger.LogInformation($"Создан новый канал. ID: {newChannelGuid}, Name: {newChannelName}");

            return newChannel;
        }

        public object CloseChannel(ChannelCommand channel_command)
        {
            if (!string.IsNullOrWhiteSpace(channel_command.Id))
            {
                if (_Channels.TryGetValue(channel_command.Id, out var channel))
                {
                    if (channel.State == "CLOSED")
                    {
                        _logger.LogWarning($"Попытка закрыть уже закрытый канал. ID: {channel.Id}");
                    }
                    else
                    {
                        channel.State = "CLOSED";
                        _logger.LogInformation($"Канал остановлен. ID: {channel.Id}");
                    }
                    return channel;
                }
                throw new ArgumentException($"Ошибка 5: Канал с id {channel_command.Id} не найден.");
            }

            var closedChannels = new List<Channel>();

            foreach (var channel in _Channels.Values)
            {
                channel.State = "CLOSED";
                channel.Reason = channel_command.Reason;
                closedChannels.Add(channel);
            }

            return closedChannels;
        }

        public object OpenChannel(ChannelCommand channel_command)
        {
            if (!string.IsNullOrWhiteSpace(channel_command.Id))
            {
                if (_Channels.TryGetValue(channel_command.Id, out var channel))
                {
                    channel.State = "ACTIVE";
                    return channel;
                }
                throw new ArgumentException($"Ошибка 5: Канал с id {channel_command.Id} не найден.");
            }

            var openChannels = new List<Channel>();

            foreach (var channel in _Channels.Values)
            {
                channel.State = "ACTIVE";
                openChannels.Add(channel);
            }

            return openChannels;
        }

        public object DelChannel(ChannelCommand channel_command)
        {
            if (!string.IsNullOrWhiteSpace(channel_command.Id))
            {
                if (_Channels.TryRemove(channel_command.Id, out var removedChannel))
                {
                    return removedChannel;
                }
                throw new ArgumentException($"Ошибка 5: Канал с id {channel_command.Id} не найден.");
            }

            var deletedChannels = new List<Channel>();
            bool isDeletingOnlyClosed = channel_command.State == "CLOSED";

            foreach (var channel in _Channels.Values)
            {
                if (isDeletingOnlyClosed && channel.State != "CLOSED")
                {
                    continue;
                }

                if (_Channels.TryRemove(channel.Id, out var removedChannel))
                {
                    deletedChannels.Add(removedChannel);
                }
            }

            return deletedChannels;
        }

        public object EnqueueChannel(ChannelCommand channel_command)
        {
            if (_Channels.TryGetValue(channel_command.Id, out var channel))
            {
                using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(_waitEnqueueTimeout));
                try
                {
                    channel.Queue.Writer.WriteAsync(channel_command.Data, cts.Token).AsTask().Wait();
                    return true;
                }
                catch (AggregateException ex) when (ex.InnerException is TaskCanceledException)
                {
                    _logger.LogWarning($"Истечение WaitEnqueue ({_waitEnqueueTimeout} сек) для канала {channel_command.Id}");
                    throw new InvalidOperationException("Ошибка: Истекло время ожидания записи в очередь.");
                }
            }
            throw new ArgumentException($"Канал с id {channel_command.Id} не найден.");
        }

        public object PeekChannel(ChannelCommand channel_command)
        {
            if (_Channels.TryGetValue(channel_command.Id, out var channel))
            {
                if (channel.Queue.Reader.TryPeek(out string message))
                {
                    return message;
                }
                throw new InvalidOperationException("Ошибка 6: Очередь канала пуста.");
            }
            else
            {
                throw new ArgumentException($"Ошибка 5: Канал с id {channel_command.Id} не найден.");
            }
        }

        public object DequeueChannel(ChannelCommand channel_command)
        {
            if (_Channels.TryGetValue(channel_command.Id, out var channel))
            {
                if (channel.Queue.Reader.TryRead(out string message))
                {
                    return message;
                }
                throw new InvalidOperationException("Ошибка 6: Очередь канала пуста.");
            }
            else
            {
                throw new ArgumentException($"Ошибка 5: Канал с id {channel_command.Id} не найден.");
            }
        }

        public Channel? GetChannel(string id)
        {
            _Channels.TryGetValue(id, out var channel);
            return channel;
        }

        public IEnumerable<Channel> GetAllChannels()
        {
            return _Channels.Values;
        }
    }
}
