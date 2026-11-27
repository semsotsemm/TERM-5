const http = require("http");
const WebSocket = require("ws");


const HTTP_PORT = 3000;
const WS_PORT = 4000;


const server = http.createServer((req, res) => {
    if (req.url === "/start" && req.method === "GET")
    {
        res.statusCode = 200;
        res.setHeader("Content-Type", "text/html; charset=utf-8");
        res.end(`
            <!DOCTYPE html>
            <html lang="ru">
            <head>
                <meta charset="UTF-8">
                <title>10-01</title>
            </head>
            <body>
                <h1>10-01</h1>
                <button onclick="startWS()">startWS</button>
                <div id="messages"></div>
            
                <script>
                    function startWS() {
                        const socket = new WebSocket("ws://localhost:4000");
                        const messagesDiv = document.getElementById("messages");
                        let n = 0;
                        let intervalId = null;
                    
                        socket.onopen = () => {
                            intervalId = setInterval(() => {
                                n++;
                                if (socket.readyState === WebSocket.OPEN) {
                                    socket.send("10-01-client: " + n);
                                }
                            }, 3000);
                    
                            setTimeout(() => {
                                clearInterval(intervalId);
                                socket.close();
                            }, 25000);
                        };
                    
                        socket.onmessage = (event) => {
                            console.log("От сервера:", event.data);
                            const p = document.createElement("p");
                            p.textContent = event.data;
                            messagesDiv.appendChild(p);
                        };
                    
                        socket.onclose = () => {
                            clearInterval(intervalId);
                            console.log("Соединение закрыто");
                        };
                    
                        socket.onerror = (err) => {
                            console.error("Ошибка WS:", err);
                        };
                    }
                </script>
            </body>
            </html>
        `);
    }
    else
    {
        res.statusCode = 400;
        res.setHeader("Content-Type", "text/plain; charset=utf-8");
        res.end("Конечная точка не определена, попробуйте get запрос на /start.");
    }
});

const wsServer = new WebSocket.Server({ port: WS_PORT });


server.listen(HTTP_PORT, () =>{
   console.log(`Сервер запущен по адресу: http://localhost:${HTTP_PORT}`);
});


wsServer.on("connection", (ws) => {
    console.log("Клиент подключился к WS-серверу");

    let lastClientN = 0;
    let k = 0;

    ws.on("message", (message) => {
        const text = message.toString("utf-8");
        console.log(`Принято от клиента: ${text}`);

        const parts = text.split(":");
        const parsedN = Number(parts[parts.length - 1].trim());
        if (!Number.isNaN(parsedN)) {
            lastClientN = parsedN;
        }
    });

    const intervalId = setInterval(() => {
        k++;
        if (ws.readyState === WebSocket.OPEN) {
            ws.send(`10-01-server: ${lastClientN}->${k}`);
        }
    }, 5000);

    ws.on("close", () => {
        clearInterval(intervalId);
        console.log("Клиент отключился от WS-сервера");
    });

    ws.on("error", (err) => {
        clearInterval(intervalId);
        console.error("Ошибка WS-соединения:", err.message);
    });
});

wsServer.on("listening", () => {
    console.log(`WS-сервер запущен по адресу: ws://localhost:${WS_PORT}`);
});

wsServer.on("error", (err) => {
    console.error("Ошибка WS-сервера:", err.message);
});