const WebSocket = require("ws");

const PORT = 5000;
const wsServer = new WebSocket.Server({ port: PORT });

wsServer.on("connection", (ws) => {
    console.log(`Клиент подключился. Активных клиентов: ${wsServer.clients.size}`);

    ws.on("message", (message) => {
        const text = message.toString("utf-8");
        console.log(`Принято: ${text}`);

        wsServer.clients.forEach((client) => {
            if (client.readyState === WebSocket.OPEN) {
                client.send(`broadcast: ${text}`);
            }
        });
    });

    ws.on("close", () => {
        console.log(`Клиент отключился. Активных клиентов: ${wsServer.clients.size}`);
    });

    ws.on("error", (err) => {
        console.error("Ошибка WS-соединения:", err.message);
    });
});

wsServer.on("listening", () => {
    console.log(`Широковещательный WS-сервер запущен: ws://localhost:${PORT}`);
});

wsServer.on("error", (err) => {
    console.error("Ошибка WS-сервера:", err.message);
});