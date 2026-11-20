const http = require("http");
const fs = require("fs");
const path = require("path");

const filePath = path.join(__dirname, "MyFile.png");
const boundary = "----NodeBoundary0907";

const fileSize = fs.statSync(filePath).size;
const header = Buffer.from(
    `--${boundary}\r\n` +
    `Content-Disposition: form-data; name="file"; filename="MyFile.png"\r\n` +
    `Content-Type: image/png\r\n\r\n`
);
const footer = Buffer.from(`\r\n--${boundary}--\r\n`);

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/07",
    method: "POST",
    headers: {
        "Content-Type": `multipart/form-data; boundary=${boundary}`,
        "Content-Length": header.length + fileSize + footer.length
    }
};

const req = http.request(options, (res) => {
    console.log(`Статус ответа: ${res.statusCode}`);

    let responseBody = "";
    res.on("data", (chunk) => {
        responseBody += chunk.toString();
    });

    res.on("end", () => {
        console.log(`Тело ответа: ${responseBody}`);
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.write(header);

const fileStream = fs.createReadStream(filePath);
fileStream.on("end", () => {
    req.end(footer);
});
fileStream.on("error", (err) => {
    console.error(`Ошибка чтения файла: ${err.message}`);
    req.destroy();
});

fileStream.pipe(req, { end: false });