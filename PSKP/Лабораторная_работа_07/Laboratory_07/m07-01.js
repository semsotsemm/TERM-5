const fs = require('fs')
const path = require('path')

const MIMETYPES = {
    ".html": "text/html",
    ".css": "text/css",
    ".js": "text/javascript",
    ".png": "image/png",
    ".docx": "application/msword",
    ".json": "application/json",
    ".xml": "application/xml",
    ".mp4": "video/mp4"
};

module.exports = function (staticDirName) {
    const staticRoot = path.resolve(__dirname, staticDirName);
    return function(req, res) {
        let pathname;
        try {
            const parsed_url = new URL(req.url, 'http://localhost');
            pathname = decodeURIComponent(parsed_url.pathname);
        } catch (err) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('Page Not Found');
            return;
        }

        if (pathname === "/")
        {
            pathname = "/index.html";
        }

        const file_path = path.resolve(staticRoot, '.' + pathname);
        const extname = String(path.extname(pathname)).toLowerCase();
        const relative_path = path.relative(staticRoot, file_path);

        // Раздаём только разрешённые файлы внутри static.
        if (pathname.includes('\0') || !Object.hasOwn(MIMETYPES, extname) || relative_path === '..' ||
            relative_path.startsWith('..' + path.sep) || path.isAbsolute(relative_path)) {
            res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
            res.end('Page Not Found');
            return;
        }

        fs.stat(file_path, (err, stats) => {
            if (err || !stats.isFile()) {
                res.writeHead(404, { 'Content-Type': 'text/plain; charset=utf-8' });
                res.end('Page Not Found');
                return;
            }

            const content_type = MIMETYPES[extname];

            res.statusCode = 200;
            res.setHeader('Content-Type', content_type);
            res.setHeader('Content-Length', stats.size);
            const read_stream = fs.createReadStream(file_path);

            read_stream.pipe(res);

            read_stream.on("error", (err) => {
                if (res.headersSent) {
                    res.destroy(err);
                    return;
                }
                res.removeHeader('Content-Length');
                res.statusCode = 404;
                res.setHeader('Content-Type', "text/plain; charset=utf-8");

                res.end('Page Not Found');
            });
        });
    }
};
