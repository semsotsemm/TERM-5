const http = require("http");

const payload = {
    __comment: "Запрос.Лабораторная работа 8/10",
    x: 1,
    y: 2,
    s: "Сообщение",
    m: ["a", "b", "c", "d"],
    o: { surname: "Иванов", name: "Иван" },
};

const body = JSON.stringify(payload);

const options = {
    host: "localhost",
    path: "/json",
    port: 5000,
    method: "POST",
    headers: {
        "Content-Type": "application/json; charset=utf-8",
        "Accept": "application/json",
        "Content-Length": Buffer.byteLength(body),
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
        try {
            const parsed = JSON.parse(data);
            console.log("Ответ сервера (JSON):", parsed);
        } catch (err) {
            console.error("Ошибка парсинга JSON:", err.message);
            console.log("Сырой ответ:", data);
        }
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса:", err.message);
});

request.end(body);