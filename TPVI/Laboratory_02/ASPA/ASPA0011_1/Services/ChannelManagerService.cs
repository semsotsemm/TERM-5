using System.Collections.Concurrent;
using ASPA0011_1.Models;

namespace ASPA0011_1.Services
{
    public class ChannelManagerService
    {
        private readonly ConcurrentDictionary<Guid, AppChannel> _channels = new();
        private readonly ILogger<ChannelManagerService> _logger;
        private readonly int _waitEnqueueSeconds;

        public ChannelManagerService(ILogger<ChannelManagerService> logger, IConfiguration config)
        {
            _logger = logger;
            _waitEnqueueSeconds = config.GetValue<int>("WaitEnqueue", 5);
        }

        public List<ChannelInfoDto> GetAll()
        {
            _logger.LogTrace("Вызов метода GetAll()");
            return _channels.Values.Select(c => c.ToDto()).ToList();
        }

        public ChannelInfoDto? GetById(Guid id)
        {
            _logger.LogTrace("Вызов метода GetById() с параметром id={Id}", id);
            if (_channels.TryGetValue(id, out var ch))
                return ch.ToDto();

            _logger.LogError("Канал {Id} не найден (404)", id);
            return null;
        }

        public ChannelInfoDto Create(CreateChannelRequest req)
        {
            _logger.LogTrace("Вызов метода Create(): name={Name}, state={State}", req.Name, req.State);
            var ch = new AppChannel
            {
                Name = req.Name,
                State = string.IsNullOrWhiteSpace(req.State) ? "ACTIVE" : req.State.ToUpper(),
                Description = req.Description
            };

            _channels[ch.Id] = ch;
            _logger.LogInformation("Создан канал {Id} ({Name}) в состоянии {State}", ch.Id, ch.Name, ch.State);
            return ch.ToDto();
        }

        public (bool Found, object? Result) UpdateState(UpdateChannelRequest req)
        {
            _logger.LogTrace("Вызов метода UpdateState(): command={Cmd}, id={Id}, reason={Reason}", req.Command, req.Id, req.Reason);
            string cmd = req.Command.ToLower();
            string targetState = cmd == "close" ? "CLOSED" : "ACTIVE";

            if (req.Id.HasValue)
            {
                if (!_channels.TryGetValue(req.Id.Value, out var ch))
                {
                    _logger.LogError("Попытка изменить состояние несуществующего канала {Id} (404)", req.Id.Value);
                    return (false, null);
                }

                ApplyStateChange(ch, cmd, targetState, req.State, req.Reason);
                return (true, ch.ToDto());
            }

            foreach (var ch in _channels.Values)
            {
                ApplyStateChange(ch, cmd, targetState, null, req.Reason);
            }

            return (true, _channels.Values.Select(c => c.ToDto()).ToList());
        }

        private void ApplyStateChange(AppChannel ch, string cmd, string targetState, string? explicitState, string? reason)
        {
            if (cmd == "open" && ch.State == "ACTIVE")
            {
                _logger.LogWarning("Команда open вызвана для уже открытого (ACTIVE) канала {Id}", ch.Id);
            }
            else if (cmd == "close" && ch.State == "CLOSED")
            {
                _logger.LogWarning("Команда close вызвана для уже закрытого (CLOSED) канала {Id}", ch.Id);
            }

            ch.State = !string.IsNullOrEmpty(explicitState) ? explicitState.ToUpper() : targetState;

            if (cmd == "close")
                _logger.LogInformation("Канал {Id} остановлен (CLOSED). Причина: {Reason}", ch.Id, reason ?? "не указана");
            else if (cmd == "open")
                _logger.LogInformation("Канал {Id} возобновлен (ACTIVE).", ch.Id);
        }

        public List<ChannelInfoDto> Delete(DeleteChannelRequest req)
        {
            _logger.LogTrace("Вызов метода Delete(): command={Cmd}, state={State}", req.Command, req.State);

            if (string.Equals(req.State, "CLOSED", StringComparison.OrdinalIgnoreCase))
            {
                var closedIds = _channels.Values.Where(c => c.State == "CLOSED").Select(c => c.Id).ToList();
                foreach (var id in closedIds)
                {
                    if (_channels.TryRemove(id, out _))
                        _logger.LogInformation("Удален закрытый канал {Id}", id);
                }
            }
            else
            {
                var allIds = _channels.Keys.ToList();
                _channels.Clear();
                foreach (var id in allIds)
                    _logger.LogInformation("Удален канал {Id}", id);
            }

            return _channels.Values.Select(c => c.ToDto()).ToList();
        }

        public async Task<(int StatusCode, object Response)> ProcessQueueAsync(QueueCommandRequest req)
        {
            _logger.LogTrace("Вызов ProcessQueueAsync(): cmd={Cmd}, id={Id}, data={Data}", req.Command, req.Id, req.Data);

            if (!_channels.TryGetValue(req.Id, out var ch))
            {
                _logger.LogError("Канал {Id} не найден при операции {Cmd} (404)", req.Id, req.Command);
                return (404, new ErrorResponse { Id = req.Id, Error = "Канал не найден" });
            }

            if (ch.State == "CLOSED")
            {
                _logger.LogError("Операция {Cmd} отклонена: канал {Id} закрыт (CLOSED)", req.Command, req.Id);
                return (200, new ErrorResponse { Id = req.Id, Error = "Канал закрыт (CLOSED)" });
            }

            string cmd = req.Command.ToLower();

            if (cmd == "enqueue")
            {
                using var cts = new CancellationTokenSource(TimeSpan.FromSeconds(_waitEnqueueSeconds));
                try
                {
                    await ch.Channel.Writer.WriteAsync(req.Data ?? string.Empty, cts.Token);
                    return (200, new QueueItemResponse { Id = ch.Id, Data = req.Data });
                }
                catch (OperationCanceledException)
                {
                    _logger.LogWarning("Истекло время ожидания WaitEnqueue ({Sec} с) для канала {Id}", _waitEnqueueSeconds, ch.Id);
                    return (200, new ErrorResponse { Id = ch.Id, Error = $"Превышено время ожидания WaitEnqueue ({_waitEnqueueSeconds} сек.)" });
                }
            }
            else if (cmd == "dequeue")
            {
                lock (ch.SyncRoot)
                {
                    if (ch.PeekBuffer.Count > 0)
                    {
                        var item = ch.PeekBuffer.Dequeue();
                        if (ch.Channel.Reader.TryRead(out var nextItem))
                            ch.PeekBuffer.Enqueue(nextItem);

                        return (200, new QueueItemResponse { Id = ch.Id, Data = item });
                    }

                    if (ch.Channel.Reader.TryRead(out var msg))
                    {
                        return (200, new QueueItemResponse { Id = ch.Id, Data = msg });
                    }
                }

                _logger.LogError("Очередь канала {Id} пуста при попытке dequeue", ch.Id);
                return (200, new ErrorResponse { Id = ch.Id, Error = "Очередь пуста" });
            }
            else if (cmd == "peek")
            {
                lock (ch.SyncRoot)
                {
                    if (ch.PeekBuffer.Count > 0)
                    {
                        return (200, new QueueItemResponse { Id = ch.Id, Data = ch.PeekBuffer.Peek() });
                    }

                    if (ch.Channel.Reader.TryRead(out var msg))
                    {
                        ch.PeekBuffer.Enqueue(msg);
                        return (200, new QueueItemResponse { Id = ch.Id, Data = msg });
                    }
                }

                _logger.LogError("Очередь канала {Id} пуста при попытке peek", req.Id);
                return (200, new ErrorResponse { Id = ch.Id, Error = "Очередь пуста" });
            }

            _logger.LogError("Неизвестная команда очереди: {Cmd}", req.Command);
            return (400, new ErrorResponse { Id = req.Id, Error = "Неизвестная команда" });
        }
    }
}