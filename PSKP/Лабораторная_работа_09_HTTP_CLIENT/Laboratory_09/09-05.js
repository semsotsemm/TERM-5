const http = require("http");
const xml2js = require("xml2js");

const xmlData =
    `<request id="28">\n` +
    `    <x value="1"/>\n` +
    `    <x value="2"/>\n` +
    `    <m value="a"/>\n` +
    `    <m value="b"/>\n` +
    `    <m value="c"/>\n` +
    `</request>`;

const options = {
    host: "127.0.0.1",
    port: 5000,
    path: "/05",
    method: "POST",
    headers: {
        "Content-Type": "application/xml; charset=utf-8",
        "Accept": "application/xml",
        "Content-Length": Buffer.byteLength(xmlData)
    }
};

const req = http.request(options, (res) => {
    console.log(`Статус ответа: ${res.statusCode}`);

    let body = "";
    res.on("data", (chunk) => {
        body += chunk.toString();
    });

    res.on("end", () => {
        console.log(`Тело ответа (XML):\n${body}\n`);

        xml2js.parseString(body, (err, result) => {
            if (err || !result || !result.response) {
                console.error("Ошибка разбора XML-ответа");
                return;
            }

            const resp = result.response;
            console.log("Разобранный XML:");
            console.log(`id: ${resp.$?.id}, request: ${resp.$?.request}`);
            console.log(`sum (${resp.sum?.[0]?.$?.element}): ${resp.sum?.[0]?.$?.result}`);
            console.log(`concat (${resp.concat?.[0]?.$?.element}): ${resp.concat?.[0]?.$?.result}`);
        });
    });
});

req.on("error", (err) => {
    console.error(`Ошибка запроса: ${err.message}`);
});

req.write(xmlData);
req.end();