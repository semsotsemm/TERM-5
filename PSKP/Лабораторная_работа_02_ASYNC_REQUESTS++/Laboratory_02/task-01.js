const http =  require("http");
const fs = require("fs");
const path = require("path");

const PORT = 5000;

const server = http.createServer((req, res) =>
{
   if (req.url === "/html" && req.method === "GET")
   {
       const file_path = path.join(__dirname, "index.html");

       fs.readFile(file_path, (err, data) =>
       {
            if(err)
            {
                res.writeHead(500, {'Content-Type': 'text/plain; charset=utf-8'});
                res.end("Файл не найден.")
                return;
            }
            else
            {
                res.writeHead(200, {'Content-Type': 'text/html; charset=utf-8'});
                res.end(data);
            }
       });
   }
   else
   {
        res.writeHead(404, {'Content-Type': 'text/plain; charset=utf-8'});
        res.end("Page not found.")
   }
})

server.listen(PORT, () =>
{
    console.log("Сервер успешно запущен на порту: " + PORT);
});