const http = require("http");
const fs = require("fs");
const path = require("path");
const db = require("./db");

const PORT = 5000;

let timer_sd = null;
let timer_sc = null;
let timer_ss = null;

let is_collecting_stats = false;
let stats = {
    start: "",
    finish: "",
    request: 0,
    commit: 0
};

function format_date(date)
{
    const year = date.getFullYear();
    const month = String(date.getMonth() + 1).padStart(2, "0");
    const day = String(date.getDate()).padStart(2, "0");
    const hours = String(date.getHours()).padStart(2, "0");
    const minutes = String(date.getMinutes()).padStart(2, "0");
    const seconds = String(date.getSeconds()).padStart(2, "0");

    // Формат даты и времени (если преподаватель требует строго только дату без времени,
    // оставь только `${year}-${month}-${day}`)
    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
}

// Считаем количество фиксаций БД во время активного сбора статистики
db.on("COMMIT", () => {
    if (is_collecting_stats)
    {
        stats.commit++;
    }
});

const server = http.createServer((req, res) => {
    const base_url = `http://${req.headers.host}`;
    const parsed_url = new URL(req.url, base_url);

    // Считаем запросы к БД во время активного сбора статистики
    if (is_collecting_stats && parsed_url.pathname === "/api/db")
    {
        stats.request++;
    }

    if (parsed_url.pathname === "/")
    {
        if (req.method === "GET")
        {
            const file_path = path.join(__dirname, "index.html");
            fs.readFile(file_path, (err, data) => {
                if (err)
                {
                    res.statusCode = 500;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(`Ошибка чтения файла: ${err}`);
                }
                else
                {
                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/html; charset=utf-8");
                    res.end(data);
                }
            });
        }
        else
        {
            res.statusCode = 405;
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            res.end("Метод обращения недопустим.");
        }
    }
    else if (parsed_url.pathname === "/api/ss")
    {
        if (req.method === "GET")
        {
            res.statusCode = 200;
            res.setHeader("Content-Type", "application/json; charset=utf-8");
            res.end(JSON.stringify(stats));
        }
        else
        {
            res.statusCode = 405;
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            res.end("Метод обращения недопустим.");
        }
    }
    else if (parsed_url.pathname === "/api/db")
    {
        switch (req.method)
        {
            case "GET":
            {
                db.emit("GET", (err, data) => {
                    if (err)
                    {
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.statusCode = 500;
                        res.end(`Ошибка на стороне сервера: ${err}`);
                    }
                    else
                    {
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.statusCode = 200;
                        res.end(JSON.stringify(data));
                    }
                });
                break;
            }
            case "POST":
            {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    try
                    {
                        const parsed_body = JSON.parse(body);
                        db.emit("POST", parsed_body, (err, data) => {
                            if (err)
                            {
                                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                                res.statusCode = 500;
                                res.end(`Ошибка на стороне сервера: ${err}`);
                            }
                            else
                            {
                                res.setHeader("Content-Type", "application/json; charset=utf-8");
                                res.statusCode = 201;
                                res.end(JSON.stringify(data));
                            }
                        });
                    }
                    catch (e)
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Некорректный JSON.");
                    }
                });
                break;
            }
            case "PUT":
            {
                let body = "";
                req.on("data", chunk => body += chunk.toString());
                req.on("end", () => {
                    try
                    {
                        const parsed_body = JSON.parse(body);
                        db.emit("PUT", parsed_body, (err, data) => {
                            if (err)
                            {
                                res.statusCode = 500;
                                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                                res.end(`Ошибка на стороне сервера: ${err}`);
                            }
                            else
                            {
                                res.statusCode = 200;
                                res.setHeader("Content-Type", "application/json; charset=utf-8");
                                res.end(JSON.stringify(data));
                            }
                        });
                    }
                    catch (e)
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Некорректный JSON.");
                    }
                });
                break;
            }
            case "DELETE":
            {
                const id = parsed_url.searchParams.get("id");

                db.emit("DELETE", id, (err, data) => {
                    if (err)
                    {
                        res.statusCode = 500;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Ошибка на стороне сервера: ${err}.`);
                    }
                    else if (!data)
                    {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Запись с id ${id} не найдена в базе данных`);
                    }
                    else
                    {
                        res.statusCode = 200;
                        res.setHeader("Content-Type", "application/json; charset=utf-8");
                        res.end(JSON.stringify(data));
                    }
                });
                break;
            }
            default:
            {
                res.statusCode = 405;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Метод обращения недопустим.");
                break;
            }
        }
    }
    else
    {
        res.statusCode = 404;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("End-point не найден.");
    }
});

// Настройка чтения команд из стандартного потока ввода (консоли)
process.stdin.setEncoding("utf-8");
process.stdin.unref(); // Чтобы открытый поток ввода не мешал серверу завершиться при sd

process.stdin.on("readable", () => {
    let chunk = null;
    while ((chunk = process.stdin.read()) !== null)
    {
        const input = chunk.trim();
        if (!input)
        {
            continue;
        }

        const parts = input.split(/\s+/);
        const command = parts[0];
        const param = parts[1] ? Number(parts[1]) : null;

        switch (command)
        {
            case "sd":
            {
                if (timer_sd)
                {
                    clearTimeout(timer_sd);
                    timer_sd = null;
                }

                if (param !== null && !isNaN(param) && param > 0)
                {
                    console.log(`[sd] Сервер будет остановлен через ${param} сек.`);
                    timer_sd = setTimeout(() => {
                        console.log("[sd] Остановка сервера...");
                        server.close(() => {
                            console.log("[sd] Сервер успешно остановлен.");
                        });
                    }, param * 1000);
                }
                else
                {
                    console.log("[sd] Остановка сервера отменена.");
                }
                break;
            }
            case "sc":
            {
                if (timer_sc)
                {
                    clearInterval(timer_sc);
                    timer_sc = null;
                }

                if (param !== null && !isNaN(param) && param > 0)
                {
                    console.log(`[sc] Запущена периодическая фиксация БД каждые ${param} сек.`);
                    timer_sc = setInterval(() => {
                        db.emit("COMMIT", () => {
                            console.log("[COMMIT] Состояние БД зафиксировано.");
                        });
                    }, param * 1000);

                    timer_sc.unref();
                }
                else
                {
                    console.log("[sc] Периодическая фиксация БД остановлена.");
                }
                break;
            }
            case "ss":
            {
                if (timer_ss)
                {
                    clearTimeout(timer_ss);
                    timer_ss = null;
                }

                if (param !== null && !isNaN(param) && param > 0)
                {
                    is_collecting_stats = true;
                    stats.start = format_date(new Date());
                    stats.finish = "";
                    stats.request = 0;
                    stats.commit = 0;

                    console.log(`[ss] Сбор статистики запущен на ${param} сек.`);

                    timer_ss = setTimeout(() => {
                        is_collecting_stats = false;
                        stats.finish = format_date(new Date());
                        timer_ss = null;
                        console.log("[ss] Сбор статистики завершен.");
                    }, param * 1000);

                    timer_ss.unref();
                }
                else
                {
                    if (is_collecting_stats)
                    {
                        is_collecting_stats = false;
                        stats.finish = format_date(new Date());
                        console.log("[ss] Сбор статистики принудительно остановлен.");
                    }
                    else
                    {
                        console.log("[ss] Сбор статистики не был запущен.");
                    }
                }
                break;
            }
            default:
            {
                console.log(`Неизвестная команда: ${command}`);
                break;
            }
        }
    }
});

server.listen(PORT, () => {
    console.log(`Сервер прослушивает порт: ${PORT}`);
});