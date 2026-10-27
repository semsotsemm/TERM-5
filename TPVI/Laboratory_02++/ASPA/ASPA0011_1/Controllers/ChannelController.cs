using System;
using System.Linq;
using ASPA0011_1.Models;
using ASPA0011_1.Services;
using Microsoft.AspNetCore.Mvc;
using Microsoft.Extensions.Logging;

namespace ASPA0011_1.Controllers
{
    [ApiController]
    public class ChannelController : ControllerBase
    {
        private readonly ChannelManager _channelManager;
        private readonly ILogger<ChannelController> _logger;

        public ChannelController(ChannelManager channelManager, ILogger<ChannelController> logger)
        {
            _channelManager = channelManager;
            _logger = logger;
        }

        [HttpGet("api/channels")]
        public IActionResult GetAll()
        {
            var channels = _channelManager.GetAllChannels();
            if (!channels.Any()) return NoContent();
            return Ok(channels);
        }

        [HttpGet("api/channels/{id}")]
        public IActionResult Get(string id)
        {
            _logger.LogTrace($"Вызов метода Get для канала {id}");

            var channel = _channelManager.GetChannel(id);
            if (channel == null)
            {
                _logger.LogError($"Канал {id} не найден (Status 404)");
                return NotFound();
            }
            return Ok(channel);
        }

        [HttpPost("api/channels")]
        public IActionResult Create([FromBody] ChannelCommand command)
        {
            try
            {
                var result = _channelManager.ProcessCommand(command);
                return StatusCode(201, result);
            }
            catch (Exception ex)
            {
                return BadRequest(new { id = command.Id, error = ex.Message });
            }
        }

        [HttpPut("api/channels")]
        public IActionResult Update([FromBody] ChannelCommand command)
        {
            try
            {
                return Ok(_channelManager.ProcessCommand(command));
            }
            catch (Exception ex)
            {
                return NotFound(new { id = command.Id, error = ex.Message });
            }
        }

        [HttpDelete("api/channels")]
        public IActionResult Delete([FromBody] ChannelCommand command)
        {
            try
            {
                return Ok(_channelManager.ProcessCommand(command));
            }
            catch (Exception ex)
            {
                return NotFound(new { id = command.Id, error = ex.Message });
            }
        }

        [HttpPost("api/queue")]
        public IActionResult QueueOperations([FromBody] ChannelCommand command)
        {
            try
            {
                return Ok(_channelManager.ProcessCommand(command));
            }
            catch (Exception ex)
            {
                return NotFound(new { id = command.Id, error = ex.Message });
            }
        }
    }
}