const http = require("http");
const { parseStringPromise, Builder } = require("xml2js");
const fs = require("fs");
const path = require("path");

const PORT = 5000;
const staticDirectory = path.join(__dirname, "static");

fs.mkdirSync(staticDirectory, { recursive: true });

const server = http.createServer((req, res) => {
    const parsed_url = new URL(req.url, `http://${req.headers.host}`);
    switch(req.method)
    {
        case "GET":
        {
            switch(parsed_url.pathname)
            {
                case "/":
                {
                    if (parsed_url.search === "")
                    {
                        res.statusCode = 200;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Успешный ответ на GET запрос.");
                        break;
                    }

                    const x_raw = parsed_url.searchParams.get("x");
                    const y_raw = parsed_url.searchParams.get("y");

                    if (x_raw == null || y_raw == null || x_raw.trim() === "" || y_raw.trim() === "")
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Ошибка: необходимо передать оба параметра.");
                        break;
                    }

                    const x = Number(x_raw);
                    const y = Number(y_raw);

                    if (!Number.isFinite(x) || !Number.isFinite(y))
                    {
                        res.statusCode = 400;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Ошибка: оба параметра должны быть числами.");
                        break;
                    }

                    res.statusCode = 200;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end(`Успех. Полученные параметры: ${x}, ${y}. Их сумма: ${x+y}`);

                    break;
                }
                case "/09-08":
                {
                    const fileName = parsed_url.searchParams.get("file") || "MyFile.png";

                    if (fileName !== "MyFile.txt" && fileName !== "MyFile.png") {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Файл не найден на сервере");
                        break;
                    }
                    const filePath = path.join(staticDirectory, fileName);

                    if (!fs.existsSync(filePath)) {
                        res.statusCode = 404;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end("Файл не найден на сервере");
                        break;
                    }

                    const stat = fs.statSync(filePath);

                    res.statusCode = 200;
                    res.setHeader("Content-Type", "application/octet-stream");
                    res.setHeader("Content-Disposition", `attachment; filename="${fileName}"`);
                    res.setHeader("Content-Length", stat.size);

                    const readStream = fs.createReadStream(filePath);
                    readStream.on("error", (err) => {
                        console.error("Ошибка чтения файла:", err.message);
                        if (!res.headersSent) {
                            res.statusCode = 500;
                            res.end("Internal Server Error");
                        } else {
                            res.destroy();
                        }
                    });

                    readStream.pipe(res);
                    break;
                }
                default:
                {
                    res.statusCode = 404;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end("Not Found");
                    break;
                }
            }
            break;
        }
        case "POST":
        {
            switch (parsed_url.pathname)
            {
                case "/":
                {
                    req.setEncoding("utf-8");
                    let body = "";

                    req.on("data", (chunk) => {
                        body += chunk.toString("utf-8");
                    });

                    req.on("end", () => {
                        const params = new URLSearchParams(body);
                        const x = params.get("x");
                        const y = params.get("y");
                        const s = params.get("s");

                        if (x === null || y === null || s === null) {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Ошибка: необходимо передать параметры x, y и s.");
                            return;
                        }

                        res.statusCode = 200;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Получены параметры: x = ${x}, y = ${y}, s = ${s}`);
                    });

                    break;
                }
                case "/json":
                {
                    req.setEncoding("utf-8");
                    let body = "";

                    req.on("data", (chunk) => {
                        body += chunk.toString("utf-8");
                    });

                    req.on("end", () => {
                        try {
                            const data = JSON.parse(body);

                            if (!Number.isFinite(data.x) || !Number.isFinite(data.y) ||  typeof data.s !== "string" ||  !Array.isArray(data.m) || typeof data.o !== "object" ||  data.o === null || Array.isArray(data.o) || typeof data.o.surname !== "string" || typeof data.o.name !== "string")
                            {
                                res.statusCode = 400;
                                res.setHeader("Content-Type", "application/json; charset=utf-8");
                                res.end(JSON.stringify({ error: "Некорректная структура JSON" }));
                                return;
                            }

                            const responseData = {
                                __comment: "Ответ.Лабораторная работа 8/10",
                                x_plus_y: data.x + data.y,
                                Concatination_s_o: `${data.s}: ${data.o.surname}, ${data.o.name}`,
                                Length_m: data.m.length,
                            };

                            res.statusCode = 200;
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.end(JSON.stringify(responseData));
                        }
                        catch (err)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "application/json; charset=utf-8");
                            res.end(JSON.stringify({ error: "Невалидный JSON" }));
                        }
                    });

                    break;
                }
                case "/xml":
                {
                    req.setEncoding("utf-8");
                    let body = "";

                    req.on("data", (chunk) => {
                        body += chunk.toString("utf-8");
                    });

                    req.on("end", async () => {
                        try
                        {
                            const parsed = await parseStringPromise(body);

                            if (!parsed || !parsed.request || !parsed.request.$)
                            {
                                res.statusCode = 400;
                                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                                res.end("Некорректная структура XML");
                                return;
                            }

                            const requestId = parsed.request.$.id;
                            const xElements = parsed.request.x || [];
                            const mElements = parsed.request.m || [];

                            if (requestId === undefined || xElements.some((element) => !element.$ || typeof element.$.value !== "string" || element.$.value.trim() === "" || !Number.isFinite(Number(element.$.value))) || mElements.some((element) => !element.$ || typeof element.$.value !== "string"))
                            {
                                res.statusCode = 400;
                                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                                res.end("Некорректные атрибуты элементов XML");
                                return;
                            }

                            const sum = xElements.reduce((acc, el) => acc + Number(el.$.value), 0);
                            const concat = mElements.map((el) => el.$.value).join("");

                            if (!Number.isFinite(sum))
                            {
                                res.statusCode = 400;
                                res.setHeader("Content-Type", "text/plain; charset=utf-8");
                                res.end("Значения элементов <x> должны быть числами");
                                return;
                            }

                            const responseXml = new Builder({ headless: true }).buildObject({
                                response: {
                                    $: { id: "33", request: requestId },
                                    sum: { $: { element: "x", result: sum } },
                                    concat: { $: { element: "m", result: concat } },
                                },
                            });

                            res.statusCode = 200;
                            res.setHeader("Content-Type", "application/xml; charset=utf-8");
                            res.end(responseXml);
                        }
                        catch (err)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Невалидный XML: " + err.message);
                        }
                    });

                    break;
                }
                case "/09-06":
                {
                    const chunks = [];

                    req.on("data", (chunk) =>
                    {
                        chunks.push(chunk);
                    });

                    req.on("end", () =>
                    {
                        const contentType = req.headers["content-type"] || "";
                        const match = contentType.match(/boundary=(.+)$/);

                        if (!match)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Ошибка: не указан boundary в Content-Type");
                            return;
                        }

                        const body = Buffer.concat(chunks);
                        const boundary = match[1];
                        const headerEndIndex = body.indexOf(Buffer.from("\r\n\r\n"));
                        const footerStartIndex = body.lastIndexOf(Buffer.from(`\r\n--${boundary}--`));

                        if (headerEndIndex === -1 || footerStartIndex < headerEndIndex + 4)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Ошибка: некорректный формат multipart/form-data");
                            return;
                        }

                        const fileBuffer = body.subarray(headerEndIndex + 4, footerStartIndex);
                        const fileText = fileBuffer.toString("utf-8");
                        const savePath = path.join(staticDirectory, "uploaded_MyFile.txt");
                        try {
                            fs.writeFileSync(savePath, fileBuffer);
                        } catch (err) {
                            console.error("Ошибка записи файла:", err.message);
                            res.statusCode = 500;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Не удалось сохранить файл на сервере");
                            return;
                        }

                        res.statusCode = 200;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Файл получен и сохранён как uploaded_MyFile.txt.\nСодержимое:\n${fileText}`);
                    });

                    break;
                }
                case "/09-07":
                {
                    const chunks = [];

                    req.on("data", (chunk) => {
                        chunks.push(chunk);
                    });

                    req.on("end", () => {
                        const contentType = req.headers["content-type"] || "";
                        const match = contentType.match(/boundary=(.+)$/);

                        if (!match)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Ошибка: не указан boundary в Content-Type");
                            return;
                        }

                        const boundary = match[1];
                        const bodyBuffer = Buffer.concat(chunks);

                        const headerEndIndex = bodyBuffer.indexOf(Buffer.from("\r\n\r\n"));
                        const footerStartIndex = bodyBuffer.lastIndexOf(Buffer.from(`\r\n--${boundary}--`));

                        if (headerEndIndex === -1 || footerStartIndex < headerEndIndex + 4)
                        {
                            res.statusCode = 400;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Ошибка: некорректный формат multipart/form-data");
                            return;
                        }

                        const fileBuffer = bodyBuffer.subarray(headerEndIndex + 4, footerStartIndex);
                        const savePath = path.join(staticDirectory, "uploaded_MyFile.png");
                        try {
                            fs.writeFileSync(savePath, fileBuffer);
                        } catch (err) {
                            console.error("Ошибка записи файла:", err.message);
                            res.statusCode = 500;
                            res.setHeader("Content-Type", "text/plain; charset=utf-8");
                            res.end("Не удалось сохранить файл на сервере");
                            return;
                        }

                        res.statusCode = 200;
                        res.setHeader("Content-Type", "text/plain; charset=utf-8");
                        res.end(`Файл MyFile.png получен и сохранён (${fileBuffer.length} байт).`);
                    });

                    break;
                }
                default:
                {
                    res.statusCode = 404;
                    res.setHeader("Content-Type", "text/plain; charset=utf-8");
                    res.end("Not Found");
                    break;
                }
            }
            break;
        }
        default:
        {
            res.statusCode = 405;
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            res.end("Method Not Allowed");
            break;
        }
    }
});

server.listen(PORT, () => {
   console.log(`Сервер запущен по адресу: http://localhost:${PORT}`);
});