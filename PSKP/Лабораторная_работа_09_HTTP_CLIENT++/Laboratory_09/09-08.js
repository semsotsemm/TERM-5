const http = require("http");
const fs = require("fs");
const path = require("path");
const { pipeline } = require("stream");

const downloadsDirectory = path.join(__dirname, "downloads");
fs.mkdirSync(downloadsDirectory, { recursive: true });

const parameters = new URLSearchParams({ file: process.argv[2] || "MyFile.png" });

const options = {
    host: "localhost",
    path: `/09-08?${parameters.toString()}`,
    port: 5000,
    method: "GET",
};

const request = http.request(options, (res) => {
    console.log("Статус код:", res.statusCode);

    if (res.statusCode !== 200)
    {
        res.setEncoding("utf-8");
        let errorBody = "";
        res.on("data", (chunk) => {
            errorBody += chunk.toString("utf-8");
        });
        res.on("end", () => {
            console.error("Ошибка сервера:", errorBody);
        });
        return;
    }

    const disposition = res.headers["content-disposition"] || "";
    const match = disposition.match(/filename="([^"]+)"/);
    const fileName = match ? `downloaded_${path.basename(match[1])}` : "downloaded_file";
    const savePath = path.join(downloadsDirectory, fileName);

    const writeStream = fs.createWriteStream(savePath);

    pipeline(res, writeStream, (err) => {
        if (err)
        {
            fs.rm(savePath, { force: true }, (removeError) => {
                if (removeError)
                {
                    console.error("Ошибка удаления неполного файла:", removeError.message);
                }
                console.error("Ошибка скачивания файла:", err.message);
            });
            return;
        }

        console.log(`Файл успешно сохранён: ${savePath}`);
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса:", err.message);
});

request.end();