const http = require('http');
const fs = require('fs');
const querystring = require('querystring');
const sendmail = require('sendmail')({
    silent: false,
    devPort: 1025,
    devHost: 'localhost'
});

const PORT = 5000;

server = http.createServer((req, res) => {
    if (req.url === '/' && req.method === 'GET')
    {
        const file_path = "./index.html";
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
    else if (req.url === '/send' && req.method === 'POST')
    {
        let body = '';

        req.on('data', chunk => {
            body += chunk.toString();
        });

        req.on("end", () => {
           const parsed_data =  querystring.parse(body);

           const sender = parsed_data.sender;
           const purpose = parsed_data.purpose;
           const message = parsed_data.message;

           sendmail({
               from: sender,
               to: purpose,
               subject: "Новое сообщение",
               text: message,
           }, (err, reply) => {
               if (err)
               {
                   console.error(err);
                   res.statusCode = 500;
                   res.end("Ошибка при отправке письма.");
               }
               else
               {
                   console.log("Письмо отправлено:", reply);
                   res.statusCode = 200;
                   res.setHeader("Content-Type", "text/plain; charset=utf-8");
                   res.end("Письмо отправлено.");
               }
           });
        });
    }
    else
    {
        res.statusCode = 404;
        res.end("Page not found");
    }
});

server.listen(PORT, () => {
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});