namespace ASPA0011_1.Models
{
    public class Message
    {
        public int SenderId { get; set; }
        public string? MessageText { get; set; }
        public DateTime CreatedAt { get; set; } = DateTime.UtcNow;
    }
}
