const http = require("http");

const PORT = 5000;

const server = http.createServer((req, res) => {
    const parsed_url = new URL(req.url, `http://${req.headers.host}`);
    switch(req.method)
    {
        case "GET":
        {
            switch(parsed_url.pathname)
            {
                case "/":
                {
                    if (parsed_url.search === "")
                    {
                        res.statusCode = 200;
                        res.setHeader("Content-Type", "text/plain");
                        res.end("Успешый ответ на GET запрос.");
                        break;
                    }

                    const x_raw = parsed_url.searchParams.get("x");
                    const y_raw = parsed_url.searchParams.get("y");

                    if (x_raw == null || y_raw == null || x_raw.trim() === "" || y_raw.trim() === "")
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Ошибка: необходимо передать оба параметра.");
                        break;
                    }

                    const x = Number(x_raw);
                    const y = Number(y_raw);

                    if (Number.isNaN(x) || Number.isNaN(y))
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Ошибка: оба параметра должны быть числами.");
                        break;
                    }

                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(`Успех. Полученные параметры: ${x}, ${y}. Их сумма: ${x+y}`);

                    break;
                }
            }
            break;
        }
        case "POST":
        {
            break;
        }
        default:
        {
            res.statusCode = 405;
            res.setHeader("Content-Type", "text/plain");
            res.end("Method Not Allowed");
            break;
        }
    }
});

server.listen(PORT, () => {
   console.log(`Сервер запущен по адресу: http://localhost:${PORT}`);
});