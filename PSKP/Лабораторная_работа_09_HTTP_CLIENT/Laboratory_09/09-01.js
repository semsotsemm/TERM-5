const http = require("http");

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/01",
    method: "GET"
};

const req = http.request(options, (res) => {
    const remoteIp = res.socket.remoteAddress;
    const remotePort = res.socket.remotePort;

    console.log(`Статус ответа: ${res.statusCode}`);
    console.log(`Сообщение к статусу: ${res.statusMessage}`);
    console.log(`IP-адрес сервера: ${remoteIp}`);
    console.log(`Порт сервера: ${remotePort}`);

    let body = "";
    res.on("data", (chunk) => {
        body += chunk.toString();
    });

    res.on("end", () => {
        console.log(`Тело ответа: ${body}`);
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.end();