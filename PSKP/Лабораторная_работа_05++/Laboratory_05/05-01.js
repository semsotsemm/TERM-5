const http = require("http");
const db = require("./db");
const fs = require("fs");
const readline = require("readline");

const PORT = 5000;

let sdTimer = null;
let scInterval = null;
let ssTimeout = null;

let isCollectingStats = false;
let stats = {
    start: "",
    finish: "",
    request: 0,
    commit: 0
};

const getFormattedDate = () => {
    return new Date().toISOString();
};

const server = http.createServer((req, res) => {
    if (isCollectingStats) {
        stats.request++;
    }

    const base_url = `http://${req.headers.host}`;
    const parsed_url = new URL(req.url, base_url);

    if (parsed_url.pathname === "/api/ss" && req.method === "GET") {
        res.setHeader("Content-Type", "application/json; charset=utf-8");
        res.statusCode = 200;
        res.end(JSON.stringify({
            start: stats.start,
            finish: isCollectingStats ? "" : stats.finish,
            request: stats.request,
            commit: stats.commit
        }));
        return;
    }

    if (parsed_url.pathname === "/api/db") {
        switch (req.method) {
            case "GET": {
                const id = parsed_url.searchParams.get("id");
                db.emit("GET", id, (err, data) => {
                    if (err) {
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.statusCode = 500;
                        res.end(`Ошибка на стороне сервера: ${err}`);
                    } else if (id && !data) {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Запись с id ${id} не найдена в базе данных`);
                    } else {
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.statusCode = 200;
                        res.end(JSON.stringify(data));
                    }
                });
                break;
            }
            case "POST": {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    const parsed_body = JSON.parse(body);
                    db.emit("POST", parsed_body, (err, data) => {
                        if (err) {
                            res.setHeader("Content-Type", "text/plain");
                            res.statusCode = 500;
                            res.end(`Ошибка на стороне сервера: ${err}`);
                        } else {
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.statusCode = 201;
                            res.end(JSON.stringify(data));
                        }
                    });
                });
                break;
            }
            case "PUT": {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    const parsed_body = JSON.parse(body);
                    db.emit("PUT", parsed_body, (err, data) => {
                        if (err) {
                            res.statusCode = 500;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end(`Ошибка на стороне сервера: ${err}`);
                        } else {
                            res.statusCode = 200;
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.end(JSON.stringify(data));
                        }
                    });
                });
                break;
            }
            case "DELETE": {
                const id = parsed_url.searchParams.get("id");
                db.emit("DELETE", id, (err, data) => {
                    if (err) {
                        res.statusCode = 500;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Ошибка на стороне сервера: ${err}.`);
                    } else if (!data) {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Запись с id ${id} не найдена в базе данных`);
                    } else {
                        res.statusCode = 200;
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.end(JSON.stringify(data));
                    }
                });
                break;
            }
            default: {
                res.statusCode = 405;
                res.end("Метод обращения недопустим.");
                break;
            }
        }
    } else if (parsed_url.pathname === "/" && req.method === "GET") {
        let file_path = "./index.html";
        fs.readFile(file_path, (err, data) => {
            if (err) {
                res.statusCode = 500;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Ошибка на стороне сервера.");
            } else {
                res.statusCode = 200;
                res.setHeader("Content-Type", "text/html; charset=utf-8");
                res.end(data);
            }
        });
    } else {
        res.statusCode = 404;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("End-point не найден.");
    }
});

server.listen(PORT, () => {
    console.log(`Сервер прослушивает порт: ${PORT}`);
    console.log('Доступные команды: sd [x], sc [x], ss [x]');
});

const line_reader = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    prompt: ''
});

line_reader.on('line', (line) => {
    const args = line.trim().split(/\s+/);
    const cmd = args[0];
    const param = args[1] ? parseInt(args[1], 10) : null;

    switch (cmd) {
        case 'sd':
            if (sdTimer) {
                clearTimeout(sdTimer);
                sdTimer = null;
            }
            if (param !== null && !isNaN(param)) {
                console.log(`Остановка сервера через ${param} секунд...`);
                sdTimer = setTimeout(() => {
                    console.log("Остановка сервера.");
                    process.exit(0);
                }, param * 1000);
            }
            else {
                console.log("Остановка сервера отменена.");
            }
            break;

        case 'sc':
            if (scInterval) {
                clearInterval(scInterval);
                scInterval = null;
            }
            if (param !== null && !isNaN(param)) {
                console.log(`Периодический комит включен каждые ${param} секунд.`);
                scInterval = setInterval(() => {
                    db.emit("COMMIT");
                    if (isCollectingStats) {
                        stats.commit++;
                    }
                }, param * 1000);
                scInterval.unref();
            }
            else {
                console.log("Периодическая фиксация БД остановлена.");
            }
            break;

        case 'ss':
            if (ssTimeout) {
                clearTimeout(ssTimeout);
                ssTimeout = null;
            }
            if (param !== null && !isNaN(param)) {
                console.log(`Сбор статистики начат. Продлится ${param} секунд.`);
                isCollectingStats = true;
                stats.start = getFormattedDate();
                stats.finish = "";
                stats.request = 0;
                stats.commit = 0;

                ssTimeout = setTimeout(() => {
                    isCollectingStats = false;
                    stats.finish = getFormattedDate();
                    console.log(`--- Статистика за ${param} сек ---`);
                    console.log(`Запросов обработано: ${stats.request}`);
                    console.log(`Комитов выполнено: ${stats.commit}`);
                    console.log(`----------------------------------`);
                }, param * 1000);
                ssTimeout.unref();
            } else {
                if (isCollectingStats) {
                    isCollectingStats = false;
                    stats.finish = getFormattedDate();
                    console.log("Сбор статистики принудительно остановлен.");
                }
            }
            break;
    }
});