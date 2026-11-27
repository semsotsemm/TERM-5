const http = require("http");
const { parseStringPromise } = require("xml2js");

const body = `
<request id="28">
    <x value="1"/>
    <x value="2"/>
    <m value="a"/>
    <m value="b"/>
    <m value="c"/>
</request>
`.trim();

const options = {
    host: "localhost",
    path: "/xml",
    port: 5000,
    method: "POST",
    headers: {
        "Content-Type": "application/xml; charset=utf-8",
        "Accept": "application/xml",
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

    res.on("end", async () => {
        console.log("Ответ сервера (XML):\n" + data);

        try {
            const parsed = await parseStringPromise(data);
            console.log("Распарсенный ответ:", JSON.stringify(parsed, null, 2));
        } catch (err) {
            console.error("Ошибка парсинга XML:", err.message);
        }
    });
});

request.on("error", (err) => {
    console.error("Ошибка запроса:", err.message);
});

request.end(body);