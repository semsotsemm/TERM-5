namespace ASPA0011_1.Models
{
    public class Channel
    {
        public string? Id { get; set; }
        public string? Name { get; set; }
        public string? Description { get; set; }
        public string? State { get; set; }
        public string? Reason { get; set; }
        public System.Threading.Channels.Channel<String> Queue { get; set; }
    }
}
