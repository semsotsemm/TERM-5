const http = require("http");

const PORT = 5000;

function PrintOperationResult(x, y)
{
    return `x=${x}; y=${y}\n\nx+y=${x+y}\nx-y=${x-y}\nx*y=${x*y}\nx/y=${x/y}\n`;
}

const server = http.createServer((req, res) => {
    const parsed_url = new URL(req.url, `http://${req.headers.host}`)

    if (req.method === "GET")
    {
        switch (parsed_url.pathname)
        {
            case "/connection":
            {
                const set_parameter = parsed_url.searchParams.get("set");
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                if (set_parameter == null)
                {
                    res.statuscode = 200;
                    res.end(`Текущее значение KeepAliveTimeout: ${server.keepAliveTimeout} мс. | Порт клиента: ${req.socket.remotePort}`);
                    break;
                }

                const new_timeout = Number(set_parameter);
                if (set_parameter.trim() === "" || !Number.isInteger(new_timeout) || new_timeout < 0)
                {
                    res.statuscode = 400;
                    res.end("Ошибка: передано невалидное значение для KeepAliveTimeout");
                    break;
                }

                server.keepAliveTimeout = new_timeout;
                res.statuscode = 200;
                res.end(`Новое значение для переменной KeepAliveTimeout: ${new_timeout}`);
                break;
            }
            case "/headers":
            {
                res.setHeader("X-Laboratory-Number", 8)
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.statuscode = 200;

                let result = "Заголовки запроса:\n";
                for (const [key, value] of Object.entries(req.headers))
                {
                    result += `${key} : ${value}\n`;
                }

                const res_headers = res.getHeaders();
                result += "\n\nЗаголовки ответа:\n";
                for (const [key, value] of Object.entries(res_headers))
                {
                    result += `${key} : ${value}\n`;
                }

                res.end(result);
                break;
            }
            case "/parameter":
            {
                const x_parameter = parsed_url.searchParams.get("x");
                const y_parameter = parsed_url.searchParams.get("y");
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                if (x_parameter == null || y_parameter == null)
                {
                    res.statuscode = 400;
                    res.end("Заданы невалидыне параметры x и/или y.");
                    break;
                }

                const x_value = Number(x_parameter);
                const y_value = Number(y_parameter);
                if (x_parameter.trim() === "" || !Number.isInteger(x_value) || y_parameter.trim() === "" || !Number.isInteger(y_value))
                {
                    res.statuscode = 400;
                    res.end("Заданы невалидыне параметры x и/или y.");
                    break;
                }

                PrintOperationResult()


                break;
            }
            case "/close":
            {
                break;
            }
            case "/socket":
            {
                break;
            }
            case "/req-data":
            {
                break;
            }
            case "/res-status":
            {
                break;
            }
            case "/files":
            {
                break;
            }
            case "/upload":
            {
                break;
            }
            case "/":
            {
                break;
            }
            default:
            {
                res.statuscode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Page Not Found");
                break;
            }
        }
    }
    else if (parsed_url.pathname === "POST")
    {
        switch (req.url)
        {
            case "/formparameter":
            {
                break;
            }
            case "/json":
            {
                break;
            }
            case "/xml":
            {
                break;
            }
            case "/upload":
            {
                break;
            }
            default:
            {
                res.statuscode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Page Not Found");
                break;
            }
        }
    }
    else
    {
        res.statuscode = 405;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("Method Not Allowed");
    }
});


server.on("connection", (socket) => {
    const start = Date.now();
    const port = socket.remotePort;
    console.log(`Открыт порт: ${port}`);

    socket.on("close", () =>{
       const lived = ((Date.now() - start) / 1000).toFixed(1);
       console.log(`Порт ${port} закрыт. Жил ${lived}`)
    });
});


server.listen(PORT, ()=>{
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});