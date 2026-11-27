const http = require("http");

const options = {
    host: "localhost",
    path: "/",
    port: 5000,
    method: "GET",
};

const request = http.request(options, (res) => {
    console.log("Статус код: " + res.statusCode);
    console.log("Пояснение к коду состояния: " + res.statusMessage);
    console.log("Адрес сервера: " + res.socket.remoteAddress);
    console.log("Порт сервера: " + res.socket.remotePort);

    let data = "";

    res.on("data", (chunk) => {
       data += chunk.toString("utf-8");
    });

    res.on("end", () => {
       console.log("Тело ответа: " + data);
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса: " + err.message);
});

request.end();