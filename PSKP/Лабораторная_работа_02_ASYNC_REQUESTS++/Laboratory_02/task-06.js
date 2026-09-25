const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = 5000;

const server = http.createServer((req, res) =>
{
    if (req.url === "/jquery" && req.method === "GET")
    {
        const filePath = path.join(__dirname, "xmlhttprequest.html");

        fs.readFile(filePath, (err, data) =>
        {
            if (err)
            {
                res.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
                res.end("Ошибка чтения файла.");
                return;
            }
            res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
            res.end(data);
        });

    }
    else if (req.url === "/api/name" && req.method === "GET")
    {
        res.writeHead(200, { "Content-Type": "text/plain; charset=utf-8" });
        res.end("Антипов Алексей Романович");

    }
    else
    {
        res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
        res.end("Page not found.");
    }
});

server.listen(PORT, () =>
{
    console.log("Сервер успешно запущен на порту: " + PORT);
});