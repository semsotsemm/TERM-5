using ASPA0011_1.Models;
using ASPA0011_1.Services;
using Microsoft.AspNetCore.Mvc;

namespace ASPA0011_1.Controllers
{
    [ApiController]
    [Route("api")]
    public class ChannelsController : ControllerBase
    {
        private readonly ChannelManagerService _service;
        private readonly ILogger<ChannelsController> _logger;

        public ChannelsController(ChannelManagerService service, ILogger<ChannelsController> logger)
        {
            _service = service;
            _logger = logger;
            _logger.LogTrace("Создан контроллер ChannelsController");
        }

        [HttpGet("channels")]
        public IActionResult GetChannels()
        {
            _logger.LogTrace("Вызван endpoint GET /api/channels");
            var list = _service.GetAll();
            if (list.Count == 0)
                return NoContent(); 

            return Ok(list); 
        }

        [HttpGet("channels/{guid:guid}")]
        public IActionResult GetChannel(Guid guid)
        {
            _logger.LogTrace("Вызван endpoint GET /api/channels/{Guid}", guid);
            var ch = _service.GetById(guid);
            if (ch == null)
                return NotFound(new ErrorResponse { Id = guid, Error = "Канал не найден" }); // 404

            return Ok(ch);
        }

        [HttpPost("channels")]
        public IActionResult CreateChannel([FromBody] CreateChannelRequest req)
        {
            _logger.LogTrace("Вызван endpoint POST /api/channels");
            var created = _service.Create(req);

            if (created.State == "CLOSED")
                return StatusCode(204, created); 

            return StatusCode(201, created); 
        }

        [HttpPut("channels")]
        public IActionResult UpdateChannels([FromBody] UpdateChannelRequest req)
        {
            _logger.LogTrace("Вызван endpoint PUT /api/channels");
            var (found, result) = _service.UpdateState(req);
            if (!found)
                return NotFound(new ErrorResponse { Id = req.Id, Error = "Канал не найден" });

            return Ok(result); 
        }

        [HttpDelete("channels")]
        public IActionResult DeleteChannels([FromBody] DeleteChannelRequest req)
        {
            _logger.LogTrace("Вызван endpoint DELETE /api/channels");
            var before = _service.GetAll();
            if (before.Count == 0)
            {
                _logger.LogError("Попытка удаления каналов при пустом списке (404)");
                return NotFound(new ErrorResponse { Error = "Каналы для удаления не найдены" }); // 404
            }

            var remaining = _service.Delete(req);
            return Ok(remaining); 
        }

        [HttpPost("queue")]
        public async Task<IActionResult> QueueOperation([FromBody] QueueCommandRequest req)
        {
            _logger.LogTrace("Вызван endpoint POST /api/queue");
            var (statusCode, response) = await _service.ProcessQueueAsync(req);
            return StatusCode(statusCode, response);
        }
    }
}