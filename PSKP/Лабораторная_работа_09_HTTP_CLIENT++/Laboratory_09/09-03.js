const http = require("http");

const body = new URLSearchParams({x:10, y:20, s:"Hello"}).toString();

const options = {
    host: "localhost",
    path: "/",
    port: 5000,
    method: "POST",
    headers: {
        "Content-Type": "application/json",
        "Content-Length": Buffer.byteLength(body),
    }
};

const request = http.request(options, (res) => {
    console.log("Статус код: ", res.statusCode);

    res.setEncoding("utf-8");
    let data = "";

    res.on("data", (chunk) => {
       data += chunk.toString("utf-8");
    });
    res.on("end", () => {
        console.log("Ответ сервера: ", data);
    });
});

request.on("error", (err) => {
    console.error(`Ошибка: ${err}`);
});

request.end(body);