import { createServer } from 'node:http';

const HOST = '127.0.0.1';
const PORT = 3000;

const server = createServer((_request, response) => {
  response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
  response.end('<h1>Hello World</h1>');
});

server.listen(PORT, HOST, () => {
  console.log(`01-02: http://${HOST}:${PORT}`);
});
