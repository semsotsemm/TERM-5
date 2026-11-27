const WebSocket = require("ws");

const clientName = process.argv[2] || `client-${process.pid}`;
const socket = new WebSocket("ws://localhost:5000");

let n = 0;
let intervalId = null;
let timeoutId = null;

socket.on("open", () => {
    console.log(`[${clientName}] Подключено к серверу`);

    intervalId = setInterval(() => {
        n++;
        if (socket.readyState === WebSocket.OPEN) {
            socket.send(`${clientName}: message ${n}`);
        }
    }, 3000);

    timeoutId = setTimeout(() => {
        clearInterval(intervalId);
        socket.close();
    }, 25000);
});

socket.on("message", (data) => {
    console.log(`[${clientName}] Принято -> ${data.toString("utf-8")}`);
});

socket.on("close", () => {
    clearInterval(intervalId);
    clearTimeout(timeoutId);
    console.log(`[${clientName}] Соединение закрыто`);
});

socket.on("error", (err) => {
    clearInterval(intervalId);
    clearTimeout(timeoutId);
    console.error(`[${clientName}] Ошибка:`, err.message);
});