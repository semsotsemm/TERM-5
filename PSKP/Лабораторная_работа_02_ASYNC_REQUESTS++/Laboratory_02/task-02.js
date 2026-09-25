const http = require("http");
const fs = require("fs");
const path = require("path");

const PORT = 5000;

const server = http.createServer((req, res) => {
    if (req.url === "/png" && req.method === "GET") {
        const filePath = path.join(__dirname, "pic.png");

        fs.readFile(filePath, (err, data) => {
            if (err)
            {
                res.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
                res.end("Ошибка чтения файла pic.png");
                return;
            }

            res.writeHead(200, { "Content-Type": "image/png" });
            res.end(data);
        });
    }
    else
    {
        res.writeHead(404, {'Content-Type': 'text/plain; charset=utf-8'});
        res.end("Page not found.")
    }
});

server.listen(PORT, () =>
{
    console.log("Сервер успешно запущен на порту: " + PORT);
});