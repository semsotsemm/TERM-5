const WebSocket = require("ws");

const socket = new WebSocket("ws://localhost:4000");

let n = 0;
let intervalId = null;
let timeoutId = null;

socket.on("open", () => {
    console.log("Подключено к WS-серверу");

    intervalId = setInterval(() => {
        n++;
        if (socket.readyState === WebSocket.OPEN) {
            socket.send(`10-01-client: ${n}`);
        }
    }, 3000);

    timeoutId = setTimeout(() => {
        clearInterval(intervalId);
        socket.close();
    }, 25000);
});

socket.on("message", (data) => {
    console.log("От сервера:", data.toString("utf-8"));
});

socket.on("close", () => {
    clearInterval(intervalId);
    clearTimeout(timeoutId);
    console.log("Соединение закрыто");
});

socket.on("error", (err) => {
    clearInterval(intervalId);
    clearTimeout(timeoutId);
    console.error("Ошибка WS:", err.message);
});