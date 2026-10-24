const http = require("node:http");
const fs = require("node:fs");
const path = require("node:path");
const { SMTPServer } = require("smtp-server");
const { simpleParser } = require("mailparser");

// Локальная почта для демонстрации. Принимает SMTP, не пересылает письма в интернет.
const messages = [];
let nextId = 1;
const smtp = new SMTPServer({
    name: "localhost",
    banner: "PSKP Lab06 local mail",
    authOptional: true,
    disabledCommands: ["AUTH", "STARTTLS"],
    size: 1024 * 1024,
    rejectOversized: true,
    onData(stream, session, callback) {
        const chunks = [];
        stream.on("data", chunk => {
            if (!stream.sizeExceeded) chunks.push(chunk);
        });
        stream.on("error", callback);
        stream.on("end", async () => {
            if (stream.sizeExceeded) return callback(new Error("Слишком большое письмо."));
            try {
                const raw = Buffer.concat(chunks);
                const mail = await simpleParser(raw);
                const message = {
                    id: nextId++, date: new Date().toISOString(),
                    from: mail.from?.text || "", to: mail.to?.text || "",
                    subject: mail.subject || "", text: mail.text || "",
                    envelope: {
                        from: session.envelope.mailFrom.address,
                        to: session.envelope.rcptTo.map(recipient => recipient.address)
                    },
                    raw: raw.toString("utf8")
                };
                messages.unshift(message);
                if (messages.length > 100) messages.pop();
                console.log(`Письмо #${message.id}: ${message.from} -> ${message.to}`);
                console.log(`Тема: ${message.subject}`);
                console.log(`Текст: ${message.text.trim()}\n`);
                callback(null, `Message ${message.id} accepted locally`);
            } catch (error) {
                callback(error);
            }
        });
    }
});

const web = http.createServer((req, res) => {
    const url = new URL(req.url, "http://localhost:1080");
    if (req.method !== "GET") {
        res.writeHead(405, { Allow: "GET" });
        return res.end();
    }
    if (url.pathname === "/api/messages") {
        res.writeHead(200, { "Content-Type": "application/json; charset=utf-8", "Cache-Control": "no-store" });
        res.end(JSON.stringify(messages.map(({ raw, ...message }) => message)));
    } else if (/^\/api\/messages\/\d+\/raw$/.test(url.pathname)) {
        const id = Number(url.pathname.split("/")[3]);
        const message = messages.find(item => item.id === id);
        res.writeHead(message ? 200 : 404, { "Content-Type": "text/plain; charset=utf-8" });
        res.end(message ? message.raw : "Письмо не найдено.");
    } else if (url.pathname === "/") {
        fs.readFile(path.join(__dirname, "inbox.html"), (error, data) => {
            res.writeHead(error ? 500 : 200, { "Content-Type": "text/html; charset=utf-8" });
            res.end(error ? "Не удалось прочитать inbox.html." : data);
        });
    } else {
        res.writeHead(404);
        res.end();
    }
});

function fail(error) {
    console.error(`Ошибка приёмника: ${error.message}`);
    process.exitCode = 1;
    smtp.close();
    web.close();
}
smtp.on("error", fail);
web.on("error", fail);
smtp.listen(1025, "127.0.0.1", () => {
    console.log("06-01: локальный SMTP-приёмник 127.0.0.1:1025");
    web.listen(1080, "127.0.0.1", () => {
        console.log("Входящие: http://localhost:1080");
        console.log("Письма хранятся в памяти до остановки приёмника. Выход: Ctrl+C.");
    });
});
