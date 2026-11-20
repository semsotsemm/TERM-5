const http = require("http");

const jsonData = JSON.stringify({
    __comment: "Запрос.Лабораторная работа 8/10",
    x: 1,
    y: 2,
    s: "Сообщение",
    m: ["a", "b", "c", "d"],
    o: { surname: "Иванов", name: "Иван" }
});

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/04",
    method: "POST",
    headers: {
        "Content-Type": "application/json; charset=utf-8",
        "Accept": "application/json",
        "Content-Length": Buffer.byteLength(jsonData)
    }
};

const req = http.request(options, (res) => {
    console.log(`Статус ответа: ${res.statusCode}`);

    let body = "";
    res.on("data", (chunk) => {
        body += chunk.toString();
    });

    res.on("end", () => {
        try {
            const parsed = JSON.parse(body);
            console.log("Тело ответа (JSON):");
            console.log(parsed);
        } catch (err) {
            console.log("Тело ответа (raw):", body);
        }
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.write(jsonData);
req.end();