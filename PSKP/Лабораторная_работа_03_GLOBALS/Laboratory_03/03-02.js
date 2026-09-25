const http = require('http');
const fs = require("fs")

const PORT = 5000;

function get_factorial(number)
{
    if (number < 1n)
    {
        return 1n;
    }
    else
    {
        return number * get_factorial(number - 1n);
    }
}

const server = http.createServer((req,res) =>
{
    const base_url = `http://${req.headers.host}`;
    const parsed_url = new URL(req.url, base_url);

    if(parsed_url.pathname === '/' && req.method  === "GET")
    {
        fs.readFile("./factorial_page.html", (err, data) => {
            if(err)
            {
                res.writeHead(500, {"Content-Type":"text/plain"});
                res.end("Ошибка чтения файла factorial.html");
            }
            else
            {
                res.writeHead(200, {"Content-Type": "text/html; charset=utf-8"});
                res.end(data);
            }
        })
    }
    else if(parsed_url.pathname === "/fact" && req.method === "GET")
    {
        try
        {
            const factorial_number = parseInt(parsed_url.searchParams.get("k"), 10);

            if(isNaN(factorial_number) || factorial_number < 0)
            {
                throw "Некорректный параметр k.";
            }

            const user_response = {
                "k": factorial_number,
                "fact": String(get_factorial(BigInt(factorial_number)))
            }
            res.statusCode = 200;
            res.setHeader("Content-Type", "application/json; charset=utf-8");

            res.end(JSON.stringify(user_response));
        }
        catch(err)
        {
            res.statusCode = 400;
            res.setHeader("Content-Type", "text/plain; charset=utf-8");
            res.end("Параметр k должен быть целым, положительным числом.");
            return;
        }
    }
    else
    {
        res.statusCode = 404;
        res.setHeader("Content-Type", "text/plain");
        res.end("Page Not Found");
    }
});

server.listen(PORT, () =>
{
    console.log(`Сервер прослушивает порт ${PORT}.`)
});