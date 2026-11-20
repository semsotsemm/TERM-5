const http = require("http");
const fs = require("fs");
const path = require("path");
const xml2js = require("xml2js");
const multiparty = require("multiparty");

const PORT = 5000;
const UPLOAD_DIR = path.join(__dirname, "uploads");
const STATIC_DIR = path.join(__dirname, "static");

fs.mkdirSync(UPLOAD_DIR, { recursive: true });
fs.mkdirSync(STATIC_DIR, { recursive: true });

// Общий обработчик загрузки файлов для заданий 06 и 07
function handleFileUpload(req, res) {
    const form = new multiparty.Form({ uploadDir: UPLOAD_DIR });

    form.parse(req, (err, fields, files) => {
        res.setHeader("Content-Type", "text/plain; charset=utf-8");

        if (err) {
            res.statusCode = 400;
            res.end(`Ошибка разбора multipart-запроса: ${err.message}`);
            return;
        }

        const fileGroup = Object.values(files)[0];
        const uploadedFile = fileGroup && fileGroup[0];

        if (!uploadedFile || !uploadedFile.originalFilename) {
            res.statusCode = 400;
            res.end("Файл не передан");
            return;
        }

        const originalName = path.basename(uploadedFile.originalFilename);
        const targetPath = path.join(UPLOAD_DIR, originalName);

        fs.rename(uploadedFile.path, targetPath, (renameErr) => {
            if (renameErr) {
                res.statusCode = 500;
                res.end("Ошибка сохранения файла на сервере");
                return;
            }

            res.statusCode = 200;
            res.end(`Файл "${originalName}" успешно получен (${uploadedFile.size} байт)`);
        });
    });
}

const server = http.createServer((req, res) => {
    const parsedUrl = new URL(req.url, `http://${req.headers.host}`);

    if (req.method === "GET") {
        switch (parsedUrl.pathname) {
            // Задание 01
            case "/01": {
                res.statusCode = 200;
                res.statusMessage = "OK";
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Ответ сервера на GET-запрос (Задание 01)");
                break;
            }

            // Задание 02
            case "/02": {
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                const xRaw = parsedUrl.searchParams.get("x");
                const yRaw = parsedUrl.searchParams.get("y");
                const x = Number(xRaw);
                const y = Number(yRaw);

                if (xRaw === null || yRaw === null || xRaw.trim() === "" || yRaw.trim() === "" || !Number.isFinite(x) || !Number.isFinite(y)) {
                    res.statusCode = 400;
                    res.end("Ошибка: параметры x и y должны быть числами");
                    break;
                }

                res.statusCode = 200;
                res.end(`x=${x}, y=${y}\nx + y = ${x + y}`);
                break;
            }

            // Задание 08
            case "/08": {
                const rawName = parsedUrl.searchParams.get("file") || "MyFile.txt";
                const filename = path.basename(rawName);
                const filePath = path.join(STATIC_DIR, filename);

                fs.stat(filePath, (err, stats) => {
                    if (err || !stats.isFile()) {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Файл "${filename}" не найден в папке static`);
                        return;
                    }

                    res.statusCode = 200;
                    res.setHeader("Content-Type", "application/octet-stream");
                    res.setHeader("Content-Disposition", `attachment; filename="${filename}"`);
                    res.setHeader("Content-Length", stats.size);

                    const stream = fs.createReadStream(filePath);
                    stream.on("error", () => {
                        if (!res.headersSent) res.statusCode = 500;
                        res.end();
                    });
                    stream.pipe(res);
                });
                break;
            }

            default: {
                res.statusCode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Not Found");
                break;
            }
        }
    } else if (req.method === "POST") {
        switch (parsedUrl.pathname) {
            // Задание 03
            case "/03": {
                let body = "";
                req.on("data", (chunk) => { body += chunk.toString(); });
                req.on("end", () => {
                    const params = new URLSearchParams(body);
                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(
                        `Получены POST-параметры:\n` +
                        `x = ${params.get("x")}\n` +
                        `y = ${params.get("y")}\n` +
                        `s = ${params.get("s")}`
                    );
                });
                break;
            }

            // Задание 04 (JSON)
            case "/04": {
                let body = "";
                req.on("data", (chunk) => { body += chunk.toString(); });
                req.on("end", () => {
                    res.setHeader("Content-Type", "application/json; charset=utf-8");
                    try {
                        const data = JSON.parse(body);
                        const response = {
                            __comment: "Ответ.Лабораторная работа 8/10",
                            x_plus_y: data.x + data.y,
                            Concatination_s_o: `${data.s}: ${data.o.surname}, ${data.o.name}`,
                            Length_m: data.m.length
                        };
                        res.statusCode = 200;
                        res.end(JSON.stringify(response, null, 2));
                    } catch (err) {
                        res.statusCode = 400;
                        res.end(JSON.stringify({ error: "Некорректный JSON или структура данных" }));
                    }
                });
                break;
            }

            // Задание 05 (XML)
            case "/05": {
                let body = "";
                req.on("data", (chunk) => { body += chunk.toString(); });
                req.on("end", () => {
                    res.setHeader("Content-Type", "application/xml; charset=utf-8");
                    xml2js.parseString(body, (err, result) => {
                        if (err || !result || !result.request) {
                            res.statusCode = 400;
                            res.end("<error>Некорректный XML</error>");
                            return;
                        }

                        const reqId = result.request.$?.id || "";
                        const xList = result.request.x || [];
                        const mList = result.request.m || [];

                        let sum = 0;
                        for (const item of xList) {
                            sum += Number(item.$?.value || 0);
                        }

                        let concat = "";
                        for (const item of mList) {
                            concat += item.$?.value || "";
                        }

                        const responseXml =
                            `<response id="33" request="${reqId}">\n` +
                            `    <sum element="x" result="${sum}" />\n` +
                            `    <concat element="m" result="${concat}" />\n` +
                            `</response>`;

                        res.statusCode = 200;
                        res.end(responseXml);
                    });
                });
                break;
            }

            // Задания 06 и 07 (загрузка MyFile.txt и MyFile.png)
            case "/06":
            case "/07": {
                handleFileUpload(req, res);
                break;
            }

            default: {
                res.statusCode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Not Found");
                break;
            }
        }
    } else {
        res.statusCode = 405;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("Method Not Allowed");
    }
});

server.listen(PORT, "127.0.0.1", () => {
    console.log(`Общий сервер ЛР 09 запущен: http://127.0.0.1:${PORT}`);
});