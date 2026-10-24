const http = require("node:http");
const fs = require("node:fs");
const path = require("node:path");
const { sendMessage } = require("./m0603/mailer");

if (process.argv.includes("--direct")) process.env.MAIL_MODE = "direct";
const direct = process.env.MAIL_MODE === "direct";

function json(res, status, body) {
    res.writeHead(status, { "Content-Type": "application/json; charset=utf-8" });
    res.end(JSON.stringify(body));
}

const server = http.createServer((req, res) => {
    const url = new URL(req.url, "http://localhost:5000");
    if (url.pathname === "/" && req.method === "GET") {
        fs.readFile(path.join(__dirname, "index.html"), (error, data) => {
            if (error) return json(res, 500, { error: "Не удалось прочитать index.html." });
            res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
            res.end(data);
        });
    } else if (url.pathname === "/api/mode" && req.method === "GET") {
        json(res, 200, { mode: direct ? "direct" : "local" });
    } else if (url.pathname === "/api/send") {
        if (req.method !== "POST") {
            res.setHeader("Allow", "POST");
            return json(res, 405, { error: "Используй POST." });
        }
        let body = "";
        let tooLarge = false;
        req.setEncoding("utf8");
        req.on("data", chunk => {
            if (tooLarge) return;
            body += chunk;
            if (Buffer.byteLength(body, "utf8") > 65536) {
                tooLarge = true;
                body = "";
                json(res, 413, { error: "Слишком большой запрос." });
            }
        });
        req.on("end", async () => {
            if (tooLarge) return;
            let data;
            try {
                data = JSON.parse(body);
            } catch {
                return json(res, 400, { error: "Некорректный JSON." });
            }
            if (!data || typeof data !== "object" || Array.isArray(data)) {
                return json(res, 400, { error: "Ожидается объект с полями from, to, message." });
            }
            try {
                const result = await sendMessage({
                    from: typeof data.from === "string" ? data.from.trim() : data.from,
                    to: typeof data.to === "string" ? data.to.trim() : data.to,
                    subject: "ПСКП — лабораторная работа 06, приложение 06-02",
                    text: data.message
                });
                json(res, 200, {
                    ...result,
                    message: direct
                        ? "Почтовый сервер принял письмо. Проверь ящик получателя и папку «Спам»."
                        : "Письмо принято локальным SMTP-сервером. Открой входящие на localhost:1080."
                });
            } catch (error) {
                const invalid = error.message.startsWith("Укажи") || error.message.startsWith("Сообщение");
                json(res, invalid ? 400 : 502, { error: error.message });
            }
        });
    } else {
        json(res, 404, { error: "Маршрут не найден." });
    }
});

server.on("error", error => {
    console.error(`Ошибка HTTP-сервера: ${error.message}`);
    process.exitCode = 1;
});
server.listen(5000, "127.0.0.1", () => {
    console.log("06-02: http://localhost:5000");
    console.log(direct
        ? "Режим: прямая отправка через MX, порт 25. Доставка зависит от почтового сервера."
        : "Режим: локальный SMTP. Приёмник: node 06-01.js; входящие: http://localhost:1080");
});
