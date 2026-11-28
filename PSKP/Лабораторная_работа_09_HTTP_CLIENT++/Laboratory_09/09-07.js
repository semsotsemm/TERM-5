const http = require("http");
const fs = require("fs");
const path = require("path");

const boundary = "----Lab09PngBoundary";
const filePath = path.join(__dirname, "static", "MyFile.png");

const headerBuf = Buffer.from(
    `--${boundary}\r\n` +
    `Content-Disposition: form-data; name="file"; filename="MyFile.png"\r\n` +
    `Content-Type: image/png\r\n\r\n`,
    "utf-8"
);

const footerBuf = Buffer.from(`\r\n--${boundary}--\r\n`, "utf-8");
const fileSize = fs.statSync(filePath).size;

const options = {
    host: "localhost",
    path: "/09-07",
    port: 5000,
    method: "POST",
    headers: {
        "Content-Type": `multipart/form-data; boundary=${boundary}`,
        "Content-Length": headerBuf.length + fileSize + footerBuf.length,
    },
};

const request = http.request(options, (res) => {
    console.log("Статус код:", res.statusCode);

    res.setEncoding("utf-8");
    let data = "";
    res.on("data", (chunk) => {
        data += chunk.toString("utf-8");
    });
    res.on("end", () => {
        console.log("Ответ сервера:", data);
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса:", err.message);
});

request.write(headerBuf);

const readStream = fs.createReadStream(filePath);

readStream.on("error", (err) => {
    console.error("Ошибка чтения файла:", err.message);
    request.destroy();
});

readStream.on("end", () => {
    request.end(footerBuf);
});

readStream.pipe(request, { end: false });