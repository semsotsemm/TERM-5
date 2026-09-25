const http = require("http")
const db = require("./db")


const PORT = 5000;


const server = http.createServer((req, res) => {

    const base_url = `http://${req.headers.host}`;
    const parsed_url = new URL(req.url, base_url);

    if (parsed_url.pathname === "/api/db")
    {
        switch (req.method)
        {
            case "GET":
            {
                db.emit("GET", (err, data) => {
                    if (err)
                    {
                        res.setHeader("Content-Type", "text/plain");
                        res.statusCode = 500;
                        res.end(`Ошибка на стороне сервера: ${err}`);
                    }
                    else
                    {
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.statusCode = 200;
                        res.end(JSON.stringify(data));
                    }
                })
                break;
            }
            case "POST":
            {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    const parsed_body = JSON.parse(body);
                    db.emit("POST", parsed_body, (err, data) => {
                        if (err)
                        {
                            res.setHeader("Content-Type", "text/plain");
                            res.statusCode = 500;
                            res.end(`Ошибка на стороне сервера: ${err}`);
                        }
                        else
                        {
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.statusCode = 201;
                            res.end(JSON.stringify(data));
                        }
                    })
                });
                break;
            }
            case "PUT":
            {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    const parsed_body = JSON.parse(body);
                    db.emit("PUT", parsed_body, (err, data) => {
                        if (err)
                        {
                            res.statusCode = 500;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end(`Ошибка на стороне сервера: ${err}`);
                        }
                        else
                        {
                            res.statusCode = 200;
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.end(JSON.stringify(data));
                        }
                    })
                });
                break;
            }
            case "DELETE":
            {
                const id = parsed_url.searchParams.get("id");

                db.emit("DELETE", id, (err, data) => {
                    if (err)
                    {
                        res.statusCode = 500;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Ошибка на стороне сервера: ${err}.`);
                    }
                    else if (!data)
                    {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Запись с id ${id} не найдена в базе данных`);
                    }
                    else
                    {
                        res.statusCode = 200;
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.end(JSON.stringify(data));
                    }
                });
                break;
            }
            default:
            {
                res.statusCode = 405;
                res.end("Метод обращения недопустим.");
                break;
            }
        }
    }
    else
    {
        res.statusCode = 404;
        res.setHeader("Content-Type", "plain/text; charset=utf-8");
        res.end("End-point не найден.");
    }
})

server.listen(PORT, () => {
    console.log(`Сервер прослушивает порт: ${PORT}`);
});