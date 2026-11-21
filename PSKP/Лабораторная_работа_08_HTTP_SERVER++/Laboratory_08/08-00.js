const http = require("http");
const fs = require("fs");
const xml2js = require("xml2js");
const multiparty = require("multiparty");
const path = require("path");


const PORT = 5000;


function isValidNumber(str) {
    return str !== null && str.trim() !== "" && Number.isFinite(Number(str));
}


function PrintOperationResult(x, y) {
    const div = y === 0 ? "деление на ноль" : x / y;
    return `x=${x}; y=${y}\n\nx+y=${x + y}\nx-y=${x - y}\nx*y=${x * y}\nx/y=${div}\n`;
}


const server = http.createServer((req, res) => {
    const parsed_url = new URL(req.url, `http://${req.headers.host}`)

    if (req.method === "GET")
    {
        if (parsed_url.pathname.startsWith("/parameter/")) {
            const parts = parsed_url.pathname.split("/");
            res.setHeader("Content-Type", "text/plain; charset=utf-8");

            if (parts.length === 4 && isValidNumber(parts[2]) && isValidNumber(parts[3]))
            {
                res.statusCode = 200;
                res.end(PrintOperationResult(Number(parts[2]), Number(parts[3])));
            }
            else
            {
                res.statusCode = 400;
                res.end(`URI: ${req.url}`);
            }
            return;
        }

        if (parsed_url.pathname.startsWith("/files/")) {
            const filename = decodeURIComponent(parsed_url.pathname.slice("/files/".length));

            if (!filename || filename.includes("/") || filename.includes("\\"))
            {
                res.statusCode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("File Not Found");
                return;
            }

            const filePath = path.join(__dirname, "static", filename);

            fs.readFile(filePath, (err, data) => {
                if (err) {
                    res.statusCode = 404;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end("File Not Found");
                    return;
                }

                const ext = path.extname(filename).toLowerCase();
                const mimeTypes = {
                    ".html": "text/html; charset=utf-8",
                    ".txt": "text/plain; charset=utf-8",
                    ".json": "application/json; charset=utf-8",
                    ".xml": "application/xml; charset=utf-8",
                    ".png": "image/png",
                    ".jpg": "image/jpeg",
                    ".jpeg": "image/jpeg",
                    ".pdf": "application/pdf"
                };

                res.statusCode = 200;
                res.setHeader("Content-Type", mimeTypes[ext] || "application/octet-stream");
                res.end(data);
            });
            return;
        }

        switch (parsed_url.pathname)
        {
            case "/connection":
            {
                const set_parameter = parsed_url.searchParams.get("set");
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                if (set_parameter == null)
                {
                    res.statusCode = 200;
                    res.end(`Текущее значение KeepAliveTimeout: ${server.keepAliveTimeout} мс. | Порт клиента: ${req.socket.remotePort}`);
                    break;
                }

                const new_timeout = Number(set_parameter);
                if (set_parameter.trim() === "" || !Number.isInteger(new_timeout) || new_timeout < 0)
                {
                    res.statusCode = 400;
                    res.end("Ошибка: передано невалидное значение для KeepAliveTimeout");
                    break;
                }

                server.keepAliveTimeout = new_timeout;
                res.statusCode = 200;
                res.end(`Новое значение для переменной KeepAliveTimeout: ${new_timeout}`);
                break;
            }
            case "/headers":
            {
                res.setHeader("X-Laboratory-Number", 8)
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.statusCode = 200;

                let result = "Заголовки запроса:\n";
                for (const [key, value] of Object.entries(req.headers))
                {
                    result += `${key} : ${value}\n`;
                }

                const res_headers = res.getHeaders();
                result += "\n\nЗаголовки ответа:\n";
                for (const [key, value] of Object.entries(res_headers))
                {
                    result += `${key} : ${value}\n`;
                }

                res.end(result);
                break;
            }
            case "/parameter":
            {
                const x_parameter = parsed_url.searchParams.get("x");
                const y_parameter = parsed_url.searchParams.get("y");
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                if (!isValidNumber(x_parameter) || !isValidNumber(y_parameter)) {
                    res.statusCode = 400;
                    res.end("Ошибка: заданы невалидные параметры x и/или y.");
                    break;
                }

                res.statusCode = 200;
                res.end(PrintOperationResult(Number(x_parameter), Number(y_parameter)));
                break;
            }
            case "/close":
            {
                res.statusCode = 200;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Сервер будет закрыт через 10 секунд.")

                setTimeout(() => {
                   console.log("Сервер останавливает свою работу...");
                   server.close(() => {
                       console.log("Сервер остановлен.");
                       process.exit(0);
                   });
                }, 10000);
                return;
            }
            case "/socket":
            {
                res.statusCode = 200;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                const client_ip = req.socket.remoteAddress.replace("::ffff:", "");
                const client_port = req.socket.remotePort;
                const server_ip = req.socket.localAddress.replace("::ffff:", "");
                const server_port = req.socket.localPort;

                res.end(`IP и порт клиента: ${client_ip}:${client_port}\nIP и порт сервера: ${server_ip}:${server_port}`);
                break;
            }
            case "/req-data":
            {
                let chunk_count = 0;
                let total_bytes = 0;
                let chunks_info = "";

                req.on("data", (chunk) => {
                   chunk_count++;
                   total_bytes += chunk.length;
                   const line = `Порция №${chunk_count}. Суммарный размер: ${total_bytes}`;
                   console.log(line);
                   chunks_info += line + '\n';
                });

                req.on("end", () => {
                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(`Всего порций: ${chunk_count}.\nОбщий размер: ${total_bytes}.\n\n${chunks_info}`);
                });
                break;
            }
            case "/resp-status":
            {
                const code = parsed_url.searchParams.get("code");
                const message = parsed_url.searchParams.get("mess") || "";

                const code_number = Number(code);
                res.setHeader("Content-Type", "text/plain; charset=utf-8");

                if (code === null || code.trim() === "" || !Number.isInteger(code_number) || code_number < 100 || code_number > 600)
                {
                    res.statusCode = 400;
                    res.end("Invalid code number (must be 100-999).");
                    break;
                }

                res.statusCode = code_number;
                res.statusMessage = message;
                res.end(`Status: ${code_number} ${message}`);
                break;
            }
            case "/files":
            {
                const staticDir = path.join(__dirname, "static");
                fs.readdir(staticDir, { withFileTypes: true }, (err, entries) => {
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");

                    if (err)
                    {
                        res.statusCode = 500;
                        res.end("Ошибка чтения директории static");
                        return;
                    }

                    const filesCount = entries.filter((entry) => entry.isFile()).length;

                    res.statusCode = 200;
                    res.setHeader("X-static-files-count", filesCount);
                    res.end(`Количество файлов в директории static: ${filesCount}`);
                });
                break;
            }
            case "/upload":
            {
                res.statusCode = 200;
                res.setHeader("Content-Type", "text/html; charset=utf-8");
                res.end(`
                    <!DOCTYPE html>
                    <html lang="ru">
                    <head>
                        <meta charset="UTF-8">
                        <title>ПСКП 8</title>
                    </head>
                    <body>
                        <form method="POST" action="/upload" enctype="multipart/form-data">
                            <input type="file" name="file" required>
                            <button type="submit">Загрузить</button>
                        </form>
                    </body>
                    </html>
                `);
                break;
            }
            case "/":
            {
                const base = `http://${req.headers.host}`;
                res.setHeader("Content-Type", "text/html; charset=utf-8");
                res.statusCode = 200;
                res.end(`
                    <!DOCTYPE html>
                    <html lang="ru">
                    <head>
                        <meta charset="UTF-8">
                        <title>Лабораторная работа 08</title>
                    </head>
                    <body>
                        <h2>Эндпоинты сервера (ЛР 08)</h2>
                        <ul>
                            <li><b>01. Connection:</b> <a href="/connection">${base}/connection</a></li>
                            <li><b>01. Connection (set):</b> <a href="/connection?set=10000">${base}/connection?set=10000</a></li>
                            <li><b>02. Headers:</b> <a href="/headers">${base}/headers</a></li>
                            <li><b>03. Parameter (query):</b> <a href="/parameter?x=10&y=5">${base}/parameter?x=10&y=5</a></li>
                            <li><b>04. Parameter (URI):</b> <a href="/parameter/10/5">${base}/parameter/10/5</a></li>
                            <li><b>05. Close (через 10 сек):</b> <a href="/close">${base}/close</a></li>
                            <li><b>06. Socket:</b> <a href="/socket">${base}/socket</a></li>
                            <li><b>07. Req-data:</b> <a href="/req-data">${base}/req-data</a></li>
                            <li><b>08. Resp-status:</b> <a href="/resp-status?code=418&mess=CustomMessage">${base}/resp-status?code=418&mess=CustomMessage</a></li>
                            <li><b>09. Formparameter:</b> <a href="/formparameter">${base}/formparameter</a></li>
                            <li><b>10. JSON:</b> ${base}/json</li>
                            <li><b>11. XML:</b> ${base}/xml</li>
                            <li><b>12. Files:</b> <a href="/files">${base}/files</a></li>
                            <li><b>13. Files/filename:</b> <a href="/files/form.html">${base}/files/form.html</a></li>
                            <li><b>14. Upload:</b> <a href="/upload">${base}/upload</a></li>
                        </ul>
                    </body>
                    </html>
                `);
                break;
            }
            case "/formparameter":
            {
                const formPath = path.join(__dirname, "static", "form.html");
                fs.readFile(formPath, (err, data) => {
                    if (err)
                    {
                        res.statusCode = 500;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Ошибка чтения файла form.html");
                        return;
                    }
                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/html; charset=utf-8");
                    res.end(data);
                });
                break;
            }
            default:
            {
                res.statusCode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Page Not Found");
                break;
            }
        }
    }
    else if (req.method === "POST")
    {
        switch (parsed_url.pathname)
        {
            case "/formparameter":
            {
                let body = "";

                req.on("data", (chunk) => {
                    body += chunk.toString();
                });

                req.on("end", () => {
                    const params = new URLSearchParams(body);
                    let result = "Полученные параметры формы:\n\n";

                    for (const [key, value] of params)
                    {
                        result += `${key}: ${value}\n`;
                    }

                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(result);
                });
                break;
            }
            case "/json":
            {
                let body = "";

                req.on("data", (chunk) => {
                    body += chunk.toString();
                });

                req.on("end", () => {
                    res.setHeader("Content-Type", "application/json; charset=utf-8");

                    try
                    {
                        const data = JSON.parse(body);

                        const response = {
                            __comment: "Ответ.Лабораторная работа 8",
                            x_plus_y: data.x + data.y,
                            Concatination_s_o: `${data.s}: ${data.o.surname}, ${data.o.name}`,
                            Length_m: data.m.length
                        };

                        res.statusCode = 200;
                        res.end(JSON.stringify(response, null, 2));
                    }
                    catch (err)
                    {
                        res.statusCode = 400;
                        res.end(JSON.stringify({ error: "Некорректный json" }));
                    }
                });
                break;
            }
            case "/xml":
            {
                let body = "";

                req.on("data", (chunk) => {
                    body += chunk.toString();
                });

                req.on("end", () => {
                    res.setHeader("Content-Type", "application/xml; charset=utf-8");

                    xml2js.parseString(body, (err, result) => {
                        if (err || !result)
                        {
                            res.statusCode = 400;
                            res.end("<error>Некорректный XML</error>");
                            return;
                        }

                        const reqId = result.request.$?.id || "";
                        const xList = result.request.x || [];
                        const mList = result.request.m || [];

                        let sum = 0;
                        for (const item of xList)
                        {
                            sum += Number(item.$?.value || 0);
                        }

                        let concat = "";
                        for (const item of mList)
                        {
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
            case "/upload":
            {
                const staticDir = path.join(__dirname, "static");
                const form = new multiparty.Form({ uploadDir: staticDir });

                form.parse(req, (err, fields, files) => {
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");

                    if (err || !files.file || files.file.length === 0) {
                        res.statusCode = 400;
                        res.end("Ошибка при загрузке файла");
                        return;
                    }

                    const uploadedFile = files.file[0];
                    const originalName = path.basename(uploadedFile.originalFilename);

                    if (!originalName) {
                        res.statusCode = 400;
                        res.end("Файл не выбран");
                        return;
                    }

                    const targetPath = path.join(staticDir, originalName);

                    fs.rename(uploadedFile.path, targetPath, (renameErr) => {
                        if (renameErr)
                        {
                            res.statusCode = 500;
                            res.end("Ошибка сохранения файла");
                            return;
                        }

                        res.statusCode = 200;
                        res.end(`Файл "${originalName}" успешно загружен в директорию static.`);
                    });
                });
                break;
            }
            default:
            {
                res.statusCode = 404;
                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                res.end("Page Not Found");
                break;
            }
        }
    }
    else
    {
        res.statusCode = 405;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("Method Not Allowed");
    }
});


server.on("connection", (socket) => {
    const start = Date.now();
    const port = socket.remotePort;
    console.log(`Открыт порт: ${port}`);

    socket.on("close", () =>{
       const lived = ((Date.now() - start) / 1000).toFixed(1);
       console.log(`Порт ${port} закрыт. Жил ${lived}`)
    });
});


server.listen(PORT, ()=>{
    console.log(`Сервер запущен: http://localhost:${PORT}`);
});