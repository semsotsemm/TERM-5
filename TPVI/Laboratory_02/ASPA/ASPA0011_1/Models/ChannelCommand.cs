using System.Collections.Generic;

namespace ASPA0011_1.Models
{
    public class ChannelCommand
    {
        public string Command { get; set; }
        public string? Reason { get; set; }
        public string? Error { get; set; }
        public string? Data { get; set; }
        public string? Id { get; set; }
        public string? Name { get; set; }
        public string? State { get; set; }
        public string? Description { get; set; }
    }
}