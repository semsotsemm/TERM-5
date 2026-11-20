const http = require("http");
const fs = require("fs");
const path = require("path");

const fileName = "MyFile.txt";
const savePath = path.join(__dirname, `downloaded_${fileName}`);

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: `/08?file=${encodeURIComponent(fileName)}`,
    method: "GET"
};

const req = http.request(options, (res) => {
    console.log(`Статус ответа: ${res.statusCode}`);
    console.log(`Content-Disposition: ${res.headers["content-disposition"] || "-"}`);

    if (res.statusCode !== 200) {
        let errorBody = "";
        res.on("data", (chunk) => {
            errorBody += chunk.toString();
        });
        res.on("end", () => {
            console.log(`Ответ сервера: ${errorBody}`);
        });
        return;
    }

    const writeStream = fs.createWriteStream(savePath);
    res.pipe(writeStream);

    writeStream.on("finish", () => {
        const size = fs.statSync(savePath).size;
        console.log(`Файл сохранён: ${savePath} (${size} байт)`);
    });

    writeStream.on("error", (err) => {
        console.error(`Ошибка записи файла: ${err.message}`);
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.end();