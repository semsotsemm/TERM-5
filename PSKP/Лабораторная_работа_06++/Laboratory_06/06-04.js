const http = require('http');
const querystring = require('querystring');
const fs = require('fs');

const m0603 = require('m0603-antipov');

const PORT = 5000;

const server = http.createServer((req, res) => {
    if (req.url === '/' && req.method === 'GET')
    {
        const file_path = "./index2.html";

        fs.readFile(file_path, (err, data) =>
        {
            if (err)
            {
                res.statusCode = 500;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end(data);
            }
            else
            {
                res.statusCode = 200;
                res.setHeader("Content-Type", "text/html; charset=utf-8");
                res.end(data);
            }
        });
    }
    else if (req.url === '/send' && req.method === 'POST') {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on('end', () => {
            const parsedData = querystring.parse(body);
            const message = parsedData.message;

            m0603.send(message);

            console.log("Письмо отправлено.");
            res.statusCode = 200;
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            res.end("Письмо отправлено.");
        });
    }
    else
    {
        res.statusCode = 404;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("Page not found");
    }
});

server.listen(PORT, () => {
    console.log(`Сервер 06-03 запущен: http://localhost:${PORT}`);
});