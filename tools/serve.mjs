import http from "node:http";
import { readFile, stat } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";
const root = fileURLToPath(new URL("../docs/", import.meta.url));
const mime = { ".html": "text/html", ".js": "text/javascript", ".json": "application/json", ".css": "text/css", ".svg": "image/svg+xml", ".woff2": "font/woff2", ".png": "image/png", ".webmanifest": "application/manifest+json" };
http.createServer(async (req, res) => {
  try {
    const url = new URL(req.url, "http://localhost");
    const file = path.resolve(root, "." + decodeURIComponent(url.pathname === "/" ? "/index.html" : url.pathname));
    if (!file.startsWith(root) || !(await stat(file)).isFile()) { res.writeHead(404).end("Not found"); return; }
    res.writeHead(200, { "Content-Type": (mime[path.extname(file)] || "application/octet-stream") + "; charset=utf-8", "Cache-Control": "no-cache" }); res.end(await readFile(file));
  } catch { res.writeHead(404).end("Not found"); }
}).listen(Number(process.env.PORT || 4184), "127.0.0.1", () => console.log("Studio: http://127.0.0.1:" + (process.env.PORT || 4184)));
