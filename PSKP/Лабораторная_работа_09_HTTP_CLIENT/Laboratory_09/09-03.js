const http = require("http");

const postData = new URLSearchParams({
    x: 10,
    y: 5,
    s: "Привет, сервер"
}).toString();

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/03",
    method: "POST",
    headers: {
        "Content-Type": "application/x-www-form-urlencoded",
        "Content-Length": Buffer.byteLength(postData)
    }
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

req.write(postData);
req.end();