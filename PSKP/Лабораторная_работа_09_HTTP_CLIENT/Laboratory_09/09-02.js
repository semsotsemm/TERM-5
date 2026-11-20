const http = require("http");

const params = new URLSearchParams({
    x: 10,
    y: 5
});

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: `/02?${params.toString()}`,
    method: "GET"
};

const req = http.request(options, (res) => {
    console.log(`Статус ответа: ${res.statusCode}`);

    let body = "";
    res.on("data", (chunk) => {
        body += chunk.toString();
    });

    res.on("end", () => {
        console.log(`Тело ответа:\n${body}`);
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.end();