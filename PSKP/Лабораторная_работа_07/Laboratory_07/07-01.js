const http = require('http');
const staticHandler = require('./m07-01')('static');

const PORT = 5000;

const server = http.createServer((req, res) => {
    if (req.method === 'GET')
    {
        staticHandler(req, res);
    }
    else
    {
        res.statusCode = 405;
        res.setHeader('Allow', 'GET');
        res.setHeader('Content-Type', 'text/plain');
        res.end("Method Not Allowed");
    }
});

server.listen(PORT, () => {
   console.log(`http://localhost:${PORT}`);
});
