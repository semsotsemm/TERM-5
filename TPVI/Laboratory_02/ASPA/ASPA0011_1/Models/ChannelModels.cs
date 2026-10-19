using System.Text.Json.Serialization;
using System.Threading.Channels;

namespace ASPA0011_1.Models
{
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

    public class QueueCommandRequest
    {
        [JsonPropertyName("command")]
        public string Command { get; set; } = string.Empty;

        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("data")]
        public string? Data { get; set; }
    }

    public class QueueItemResponse
    {
        [JsonPropertyName("id")]
        public Guid Id { get; set; }

        [JsonPropertyName("error")]
        public string Error { get; set; } = string.Empty;
    }

    public class AppChannel
    {
        public Guid Id { get; set; } = Guid.NewGuid();
        public string Name { get; set; } = string.Empty;
        public string State {  get; set; } = "ACTIVE";
        public string Description { get; set; } = string.Empty;

        public Channel<string> Channel { get; } = System.Threading.Channels.Channel.CreateBounded<string>(
            new BoundedChannelOptions(5) 
            {
                FullMode = BoundedChannelFullMode.Wait
            });

        public readonly object SyncRoot = new();
        public Queue<string> PeekBuffer { get; } = new();

        public ChannelInfoDto ToDo() => new()
        { 
            Id = Id,
            Name = Name,
            State = State,
            Description = Description
        };
    }
}