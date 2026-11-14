const http = require('http');
const fs = require('fs/promises');
const path = require('path');
const { DOMParser, DOMImplementation, XMLSerializer } = require('@xmldom/xmldom');

const PORT = 5000;
const STATIC_PATH = path.join(__dirname, "static");
const MAX_BODY_SIZE = 10 * 1024 * 1024;

const file_types = {
    ".txt": "text/plain; charset=utf-8",
    ".html": "text/html; charset=utf-8",
    ".json": "application/json; charset=utf-8",
    ".xml": "application/xml; charset=utf-8",
    ".png": "image/png",
    ".jpg": "image/jpeg",
    ".jpeg": "image/jpeg",
    ".gif": "image/gif",
    ".pdf": "application/pdf",
    ".mp4": "video/mp4"
};

let close_timer;
let next_xml_id = 33;

function send_text(res, status_code, message) {
    res.writeHead(status_code, { "Content-Type": "text/plain; charset=utf-8" });
    res.end(message);
}

function request_error(status_code, message) {
    return Object.assign(new Error(message), { status_code });
}

async function send_page(res, file_name) {
    const data = await fs.readFile(path.join(__dirname, file_name));

    res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    res.end(data);
}

async function read_body(req, on_chunk) {
    const chunks = [];
    let size = 0;

    for await (const chunk of req.iterator({ destroyOnReturn: false })) {
        size += chunk.length;

        if (size > MAX_BODY_SIZE) {
            throw request_error(413, "Слишком большой запрос.");
        }

        if (on_chunk) {
            on_chunk(chunk);
        }
        else {
            chunks.push(chunk);
        }
    }

    return on_chunk ? size : Buffer.concat(chunks, size);
}

function handle_connection(res, value) {
    if (value === null) {
        send_text(res, 200, `KeepAliveTimeout = ${server.keepAliveTimeout} мс`);
        return;
    }

    const timeout = Number(value);

    if (!/^\d+$/.test(value) || !Number.isSafeInteger(timeout) || timeout > 2147483647) {
        throw request_error(400, "Укажите целое неотрицательное время в миллисекундах.");
    }

    server.keepAliveTimeout = timeout;
    send_text(res, 200, `Установлено новое значение параметра KeepAliveTimeout = ${timeout} мс.`);
}

function show_headers(req, res) {
    const request_headers = [];

    for (let i = 0; i < req.rawHeaders.length; i += 2) {
        request_headers.push(`${req.rawHeaders[i]}: ${req.rawHeaders[i + 1]}`);
    }

    res.useChunkedEncodingByDefault = false;
    res.setHeader("Content-Type", "text/plain; charset=utf-8");
    res.setHeader("Date", new Date().toUTCString());
    res.setHeader("Connection", "close");
    res.setHeader("X-Lab-Number", "08");

    const response_headers = Object.entries(res.getHeaders())
        .map(([name, value]) => `${name}: ${value}`);

    res.flushHeaders();
    res.end([
        "Заголовки запроса:",
        ...request_headers,
        "",
        "Заголовки ответа:",
        ...response_headers
    ].join("\n"));
}

function calculate(res, x_value, y_value, invalid_message, invalid_status) {
    const x = Number(x_value);
    const y = Number(y_value);

    if (!x_value?.trim() || !y_value?.trim() || !Number.isFinite(x) || !Number.isFinite(y)) {
        send_text(res, invalid_status, invalid_message);
        return;
    }

    send_text(res, 200, [
        `Сумма: ${x + y}`,
        `Разность: ${x - y}`,
        `Произведение: ${x * y}`,
        `Частное: ${y === 0 ? "деление на ноль" : x / y}`
    ].join("\n"));
}

function close_server(res) {
    if (!close_timer) {
        close_timer = setTimeout(() => {
            server.close();
            server.closeAllConnections();
            console.log("Сервер остановлен.");
        }, 10000);
    }

    send_text(res, 200, "Сервер будет остановлен через 10 секунд.");
}

function show_socket(req, res) {
    const socket = req.socket;

    send_text(res, 200, [
        `IP-адрес клиента: ${socket.remoteAddress}`,
        `Порт клиента: ${socket.remotePort}`,
        `IP-адрес сервера: ${socket.localAddress}`,
        `Порт сервера: ${socket.localPort}`
    ].join("\n"));
}

async function show_request_data(req, res) {
    const chunks = [];

    const size = await read_body(req, chunk => {
        const message = `Порция ${chunks.length + 1}: ${chunk.length} байт`;
        chunks.push(message);
        console.log(message);
    });

    send_text(res, 200, [
        `Получено порций: ${chunks.length}`,
        ...chunks,
        `Всего получено: ${size} байт`
    ].join("\n"));
}

function send_status(res, params) {
    const code = Number(params.get("code"));
    const message = params.get("mess");

    if (!Number.isInteger(code) || code < 200 || code > 599 || !message || !/^[\x20-\x7e\xa0-\xff]+$/.test(message)) {
        throw request_error(400, "Укажите код HTTP от 200 до 599 и допустимое пояснение.");
    }

    res.writeHead(code, message, { "Content-Type": "text/plain; charset=utf-8" });
    res.end(`${code} ${message}`);
}

async function handle_form(req, res) {
    const body = await read_body(req);
    const params = new URLSearchParams(body.toString("utf8"));
    const values = Array.from(params, ([name, value]) => `${name}: ${value}`);

    send_text(res, 200, values.join("\n"));
}

async function handle_json(req, res) {
    const body = await read_body(req);
    let data;

    try {
        data = JSON.parse(body.toString("utf8"));
    }
    catch (error) {
        throw request_error(400, "Некорректный JSON.");
    }

    if (!data || !Number.isFinite(data.x) || !Number.isFinite(data.y) ||
        typeof data.s !== "string" || !Array.isArray(data.m) ||
        typeof data.o?.surname !== "string" || typeof data.o?.name !== "string") {
        throw request_error(400, "Передайте x, y, s, m и объект o с полями surname и name.");
    }

    const result = {
        "__comment": " Ответ.Лабораторная работа 8/10",
        "x_plus_y": data.x + data.y,
        "Concatination_s_o": `${data.s}: ${data.o.surname}, ${data.o.name}`,
        "Length_m": data.m.length
    };

    res.writeHead(200, { "Content-Type": "application/json; charset=utf-8" });
    res.end(JSON.stringify(result, null, 4));
}

async function handle_xml(req, res) {
    const body = await read_body(req);
    let document;

    try {
        const parser = new DOMParser({
            onError(level, message) {
                throw new Error(message);
            }
        });

        document = parser.parseFromString(body.toString("utf8"), "application/xml");
    }
    catch (error) {
        throw request_error(400, "Некорректный XML.");
    }

    const request = document.documentElement;

    if (document.doctype || request?.tagName !== "request" || !request.getAttribute("id")) {
        throw request_error(400, "Ожидается элемент request с атрибутом id.");
    }

    let sum = 0;
    let concat = "";

    for (const element of Array.from(request.childNodes)) {
        if (element.nodeType !== 1) {
            continue;
        }

        const value = element.getAttribute("value");

        if (element.tagName === "x" && value?.trim() && Number.isFinite(Number(value))) {
            sum += Number(value);
        }
        else if (element.tagName === "m" && element.hasAttribute("value")) {
            concat += value;
        }
        else {
            throw request_error(400, "Ожидаются элементы x и m с допустимыми атрибутами value.");
        }
    }

    const result = new DOMImplementation().createDocument(null, "response", null);
    const response = result.documentElement;
    response.setAttribute("id", String(next_xml_id++));
    response.setAttribute("request", request.getAttribute("id"));

    for (const [tag, element, value] of [["sum", "x", sum], ["concat", "m", concat]]) {
        const child = result.createElement(tag);
        child.setAttribute("element", element);
        child.setAttribute("result", String(value));
        response.appendChild(child);
    }

    res.writeHead(200, { "Content-Type": "application/xml; charset=utf-8" });
    res.end(new XMLSerializer().serializeToString(result));
}

async function show_file_count(res) {
    const entries = await fs.readdir(STATIC_PATH, { withFileTypes: true });
    const count = entries.filter(entry => entry.isFile()).length;

    res.setHeader("X-static-files-count", count);
    send_text(res, 200, `Количество файлов в директории static: ${count}`);
}

function is_file_name(file_name) {
    return Boolean(file_name) && !/[<>:"/\\|?*\x00-\x1f]|[. ]$/.test(file_name) &&
        !/^(con|prn|aux|nul|com[1-9¹²³]|lpt[1-9¹²³])(?:\.|$)/i.test(file_name);
}

async function send_file(res, file_name) {
    if (!is_file_name(file_name)) {
        throw request_error(404, "Файл не найден.");
    }

    const file_path = path.join(STATIC_PATH, file_name);
    const info = await fs.lstat(file_path).catch(error => {
        if (error.code === "ENOENT") {
            return null;
        }

        throw error;
    });

    if (!info?.isFile()) {
        throw request_error(404, "Файл не найден.");
    }

    const data = await fs.readFile(file_path);
    const extension = path.extname(file_name).toLowerCase();
    const content_type = file_types[extension] || "application/octet-stream";

    res.writeHead(200, { "Content-Type": content_type });
    res.end(data);
}

async function upload_file(req, res) {
    const content_type = req.headers["content-type"] || "";

    if (!/^multipart\/form-data(?:;|$)/i.test(content_type)) {
        throw request_error(415, "Для загрузки файла используется multipart/form-data.");
    }

    const body = await read_body(req);
    let form;

    try {
        const request = new Request("http://localhost/upload", {
            method: "POST",
            headers: { "Content-Type": content_type },
            body
        });

        form = await request.formData();
    }
    catch (error) {
        throw request_error(400, "Некорректная форма загрузки.");
    }

    const file = form.get("file");

    if (!(file instanceof File) || !is_file_name(file.name)) {
        throw request_error(400, "Выберите файл с допустимым именем.");
    }

    try {
        const data = Buffer.from(await file.arrayBuffer());
        await fs.writeFile(path.join(STATIC_PATH, file.name), data, { flag: "wx" });
    }
    catch (error) {
        if (error.code === "EEXIST") {
            throw request_error(409, "Файл с таким именем уже существует.");
        }

        throw error;
    }

    send_text(res, 201, `Файл ${file.name} сохранён в директории static.`);
}

const server = http.createServer({ keepAliveTimeoutBuffer: 0 }, async (req, res) => {
    try {
        const parsed_url = new URL(req.url, "http://localhost");
        const pathname = parsed_url.pathname;
        const parameters = /^\/parameter\/([^/]+)\/([^/]+)$/.exec(pathname);
        const file = /^\/files\/([^/]+)$/.exec(pathname);

        if (req.method === "GET" && pathname === "/") {
            await send_page(res, "index.html");
        }
        else if (req.method === "GET" && (pathname === "/connection" || pathname.startsWith("/connection/set="))) {
            const value = pathname === "/connection"
                ? parsed_url.searchParams.get("set")
                : decodeURIComponent(pathname.slice("/connection/set=".length));

            handle_connection(res, value);
        }
        else if (req.method === "GET" && pathname === "/headers") {
            show_headers(req, res);
        }
        else if (req.method === "GET" && pathname === "/parameter") {
            calculate(res, parsed_url.searchParams.get("x"), parsed_url.searchParams.get("y"),
                "Параметры x и y должны быть числами.", 400);
        }
        else if (req.method === "GET" && parameters) {
            calculate(res, decodeURIComponent(parameters[1]), decodeURIComponent(parameters[2]), req.url, 200);
        }
        else if (req.method === "GET" && pathname === "/close") {
            close_server(res);
        }
        else if (req.method === "GET" && pathname === "/socket") {
            show_socket(req, res);
        }
        else if (pathname === "/req-data" && (req.method === "GET" || req.method === "POST")) {
            await show_request_data(req, res);
        }
        else if (req.method === "GET" && pathname === "/resp-status") {
            const params = new URLSearchParams(parsed_url.search.slice(1).replace("?mess=", "&mess="));
            send_status(res, params);
        }
        else if (req.method === "GET" && pathname === "/formparameter") {
            await send_page(res, "formparameter.html");
        }
        else if (req.method === "POST" && pathname === "/formparameter") {
            await handle_form(req, res);
        }
        else if (req.method === "POST" && pathname === "/json") {
            await handle_json(req, res);
        }
        else if (req.method === "POST" && pathname === "/xml") {
            await handle_xml(req, res);
        }
        else if (req.method === "GET" && pathname === "/files") {
            await show_file_count(res);
        }
        else if (req.method === "GET" && file) {
            await send_file(res, decodeURIComponent(file[1]));
        }
        else if (req.method === "GET" && pathname === "/upload") {
            await send_page(res, "upload.html");
        }
        else if (req.method === "POST" && pathname === "/upload") {
            await upload_file(req, res);
        }
        else {
            send_text(res, 404, "Страница не найдена.");
        }
    }
    catch (error) {
        req.resume();

        if (!res.headersSent && !res.destroyed) {
            if (error instanceof URIError || error.code === "ERR_INVALID_URL") {
                send_text(res, 400, "Некорректный адрес запроса.");
            }
            else if (error.status_code) {
                send_text(res, error.status_code, error.message);
            }
            else {
                console.error(error);
                send_text(res, 500, "Ошибка сервера.");
            }
        }
    }
});

server.listen(PORT, "127.0.0.1", () => {
    console.log(`Сервер прослушивает порт ${PORT}.`);
});
