const http = require("http");

const PORT = 5000;

const server = http.createServer((req, res) => {
    if (req.url === "/api/name" && req.method === "GET")
    {
        res.writeHead(200, { "Content-Type": "text/plain; charset=utf-8" });
        res.end("Антиов Алексей Романович");
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