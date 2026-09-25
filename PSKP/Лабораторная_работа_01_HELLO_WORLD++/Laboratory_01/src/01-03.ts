import { createServer, IncomingHttpHeaders } from 'node:http';

const HOST = '127.0.0.1';
const PORT = 3001;


function renderHeaders(headers: IncomingHttpHeaders): string {
  return Object.entries(headers)
    .map(([name, value]) => `<tr><td>${name}</td><td>${String(value)}</td></tr>`)
    .join('');
}

function formatBody(body: string, contentType: string): string {
  if (!body) {
    return '<em>Тело запроса отсутствует</em>';
  }

  if (contentType.includes('application/json')) {
    try {
      return `<pre>${JSON.stringify(JSON.parse(body))}</pre>`;
    } catch {
      return `<pre>${body}</pre><p><em>Получен некорректный JSON</em></p>`;
    }
  }

  return `<pre>${body}</pre>`;
}

const server = createServer((request, response) => {
  const chunks: Buffer[] = [];

  request.on('data', (chunk: Buffer) => {
    chunks.push(chunk);
  });

  request.on('end', () => {
    const method = request.method ?? '';
    const uri = request.url ?? '';
    const httpVersion = request.httpVersion;
    const body = Buffer.concat(chunks).toString('utf8');
    const contentType = request.headers['content-type'] ?? '';

    const page = `<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <title>Содержимое HTTP-запроса</title>
  <style>
    body { max-width: 900px; margin: 40px auto; font: 16px/1.5 Arial, sans-serif; }
    table { width: 100%; border-collapse: collapse; }
    th, td { border: 1px solid #bbb; padding: 8px; text-align: left; }
    th { background: #eee; }
    pre { padding: 16px; overflow-x: auto; border-radius: 8px; background: #f4f4f4; white-space: pre-wrap; word-break: break-word; }
  </style>
</head>
<body>
  <h1>Содержимое HTTP-запроса</h1>
  <p><strong>Метод:</strong> ${method}</p>
  <p><strong>URI:</strong> ${uri}</p>
  <p><strong>Версия HTTP:</strong> ${httpVersion}</p>
  <p><strong>Удалённый адрес:</strong> ${request.socket.remoteAddress ?? ''}</p>
  <h2>Тело запроса</h2>
  ${formatBody(body, contentType)}
  <h2>Заголовки</h2>
  <table><thead><tr><th>Имя</th><th>Значение</th></tr></thead><tbody>${renderHeaders(request.headers)}</tbody></table>
</body>
</html>`;

    response.writeHead(200, { 'Content-Type': 'text/html; charset=utf-8' });
    response.end(page);
  });
});

server.listen(PORT, HOST, () => {
  console.log(`01-03: http://${HOST}:${PORT}`);
});
