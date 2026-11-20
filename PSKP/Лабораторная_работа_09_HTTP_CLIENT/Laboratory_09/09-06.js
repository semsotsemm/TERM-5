const http = require("http");
const fs = require("fs");
const path = require("path");

const filePath = path.join(__dirname, "MyFile.txt");
const boundary = "----NodeBoundary0906";

const fileContent = fs.readFileSync(filePath);
const header = Buffer.from(
    `--${boundary}\r\n` +
    `Content-Disposition: form-data; name="file"; filename="MyFile.txt"\r\n` +
    `Content-Type: text/plain; charset=utf-8\r\n\r\n`
);
const footer = Buffer.from(`\r\n--${boundary}--\r\n`);
const body = Buffer.concat([header, fileContent, footer]);

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/06",
    method: "POST",
    headers: {
        "Content-Type": `multipart/form-data; boundary=${boundary}`,
        "Content-Length": body.length
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

req.write(body);
req.end();