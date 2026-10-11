const http = require('http');
const fs = require('fs');
const path = require('path');
const readline = require('readline');

const PORT = 5000;
const __available_commands = new Set(["norm", "stop", "test", "idle", "exit"]);
let last_command = "norm";
const line_reader  = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    prompt: `${last_command}->`
});


const server = http.createServer((req, res) => {
    if(req.url === '/' && req.method === "GET")
    {
        const file_path = path.join(__dirname, "index.html");

        fs.readFile(file_path, (err, data) =>
        {
            if(err)
            {
                res.statusCode = 500;
                res.setHeader('Content-Type', 'text/plain; charset=utf-8');
                res.end("Не удалось найти  необходимый файл.");
                return;
            }
            res.statusCode = 200;
            res.setHeader('Content-Type', 'text/html; charset=utf-8');
            let final_html = data.toString();
            final_html = final_html.replace("Ожидание состояния...", last_command);
            res.end(final_html);
        });

    }
    else
    {
        res.statusCode = 404;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end('Page not found');
    }
});

server.listen(PORT,() => {
    console.log(`Сервер прослушивает порт: ${PORT}`);
    line_reader.prompt();
});


line_reader.on('line', (input) =>
{
    if(__available_commands.has(input.trim().toLowerCase()))
    {
        if(input.trim().toLowerCase() === "exit")
        {
            console.log("Остановка сервера...");
            process.exit(0);
        }
        console.log(`reg = ${last_command} --> ${input.trim().toLowerCase()}`);
        last_command = input.trim().toLowerCase();
        line_reader.setPrompt(`${last_command}->`);
    }
    line_reader.prompt();
});