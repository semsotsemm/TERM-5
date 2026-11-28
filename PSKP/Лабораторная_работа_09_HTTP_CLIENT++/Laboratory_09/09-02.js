const http = require("http");

const parameters = new URLSearchParams({x: 10, y: 20});

const options = {
    host : "localhost",
    path: `/?${parameters.toString()}`,
    port: 5000,
    method: "GET",
};

const request = http.request(options, (res) => {
    console.log("Статус код: ", res.statusCode);
    console.log("Пояснение к коду состояния: ", res.statusMessage);

    res.setEncoding("utf-8");
    let data = "";

    res.on("data", chunk => {
        data += chunk.toString();
    });

    res.on("end", () => {
        console.log(`Ответ сервера: ${data}`)
    });
});

request.on("error", (err) => {
    console.error("Ошибка: ", err.message)
});

request.end();