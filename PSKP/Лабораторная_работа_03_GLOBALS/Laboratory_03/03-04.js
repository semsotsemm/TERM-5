const http = require('http');
const fs = require("fs");

const PORT = 5000;

function get_factorial_async(number, callback) {
    if (number <= 1n) {
        process.nextTick(() => {
            callback(1n);
        });
    } else {
        process.nextTick(() => {
            get_factorial_async(number - 1n, (result) => {
                callback(number * result);
            });
        });
    }
}

const server = http.createServer((req, res) => {
    const base_url = `http://${req.headers.host || 'localhost:5000'}`;
    const parsed_url = new URL(req.url, base_url);

    if (parsed_url.pathname === '/' && req.method === "GET") {
        fs.readFile("./factorial_page.html", (err, data) => {
            if (err) {
                res.writeHead(500, { "Content-Type": "text/plain; charset=utf-8" });
                res.end("Ошибка чтения файла HTML.");
            } else {
                res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
                res.end(data);
            }
        });
    }
    else if (parsed_url.pathname === "/fact" && req.method === "GET") {
        try {
            const factorial_number = parseInt(parsed_url.searchParams.get("k"), 10);

            if (isNaN(factorial_number) || factorial_number < 0) {
                throw new Error("Некорректный параметр k.");
            }

            // 2. Вызываем асинхронную функцию и передаем ей коллбэк,
            // который сработает, когда факториал будет вычислен
            get_factorial_async(BigInt(factorial_number), (fact_result) => {
                const user_response = {
                    "k": factorial_number,
                    "fact": String(fact_result) // Преобразуем BigInt в строку
                };

                res.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });
                res.end(JSON.stringify(user_response));
            });

        } catch (err) {
            res.writeHead(400, { "Content-Type": "text/plain; charset=utf-8" });
            res.end("Параметр k должен быть целым, положительным числом.");
        }
    }
    else {
        res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
        res.end("Page Not Found");
    }
});

server.listen(PORT, () => {
    console.log(`Сервер прослушивает порт ${PORT}.`);
});