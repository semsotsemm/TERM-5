using System;
using System.Collections.Generic;
using System.Text.Json.Serialization;
using System.Threading.Channels;

namespace ASPA0011_1.Models
{
    // JSON-1: Информация о канале
    public class ChannelInfoDto
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("name")]
        public string Name { get; set; } = string.Empty;

        [JsonPropertyName("state")]
        public string State { get; set; } = "ACTIVE";

        [JsonPropertyName("description")]
        public string Description { get; set; } = string.Empty;
    }

    // JSON-2: POST - создание канала
    public class CreateChannelRequest
    {
        [JsonPropertyName("command")]
        public string Command { get; set; } = string.Empty;

        [JsonPropertyName("name")]
        public string Name { get; set; } = string.Empty;

        [JsonPropertyName("state")]
        public string State { get; set; } = "ACTIVE";

        [JsonPropertyName("description")]
        public string Description { get; set; } = string.Empty;
    }

    // JSON-3, 4, 5, 6: PUT - изменение состояния каналов
    public class UpdateChannelRequest
    {
        [JsonPropertyName("command")]
        public string Command { get; set; } = string.Empty;

        [JsonPropertyName("id")]
        public Guid? Id { get; set; }

        [JsonPropertyName("reason")]
        public string? Reason { get; set; }

        [JsonPropertyName("state")]
        public string? State { get; set; }
    }

    // JSON-7, 8: DELETE - удаление каналов (которого не хватало!)
    public class DeleteChannelRequest
    {
        [JsonPropertyName("command")]
        public string Command { get; set; } = string.Empty;

        [JsonPropertyName("state")]
        public string? State { get; set; }
    }

    // JSON-9, 10: POST /api/queue - операции с очередью
    public class QueueCommandRequest
    {
        [JsonPropertyName("command")]
        public string Command { get; set; } = string.Empty;

        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("data")]
        public string? Data { get; set; }
    }

    // JSON-11: Элемент очереди (со свойством Data!)
    public class QueueItemResponse
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("data")]
        public string? Data { get; set; }
    }

    // JSON-12: Сообщение об ошибке (которого не хватало!)
    public class ErrorResponse
    {
        [JsonPropertyName("id")]
        public Guid? Id { get; set; }

        [JsonPropertyName("error")]
        public string Error { get; set; } = string.Empty;
    }

    // Внутреннее представление канала
    public class AppChannel
    {
        public Guid Id { get; set; } = Guid.NewGuid();
        public string Name { get; set; } = string.Empty;
        public string State { get; set; } = "ACTIVE";
        public string Description { get; set; } = string.Empty;

        public Channel<string> Channel { get; } = System.Threading.Channels.Channel.CreateBounded<string>(
            new BoundedChannelOptions(5)
            {
                FullMode = BoundedChannelFullMode.Wait
            });

        public readonly object SyncRoot = new();
        public Queue<string> PeekBuffer { get; } = new();

        public ChannelInfoDto ToDto() => new()
        {
            Id = Id,
            Name = Name,
            State = State,
            Description = Description
        };
    }
}