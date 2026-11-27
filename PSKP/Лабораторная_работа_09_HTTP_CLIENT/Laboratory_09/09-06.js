const http = require("http");
const fs = require("fs");
const path = require("path");

const boundary = "----Lab09Boundary";
const filePath = path.join(__dirname, "static", "MyFile.txt");
const fileContent = fs.readFileSync(filePath);

const header =
    `--${boundary}\r\n` +
    `Content-Disposition: form-data; name="file"; filename="MyFile.txt"\r\n` +
    `Content-Type: text/plain; charset=utf-8\r\n\r\n`;

const footer = `\r\n--${boundary}--\r\n`;

const body = Buffer.concat([
    Buffer.from(header, "utf-8"),
    fileContent,
    Buffer.from(footer, "utf-8"),
]);

const options = {
    host: "localhost",
    path: "/09-06",
    port: 5000,
    method: "POST",
    headers: {
        "Content-Type": `multipart/form-data; boundary=${boundary}`,
        "Content-Length": body.length,
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
        console.log("Ответ сервера:\n" + data);
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса:", err.message);
});

request.end(body);