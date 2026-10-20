// Minimal service worker: it exists so browsers offer "install as app". It deliberately caches
// nothing and passes every request through, so the build-pinned (?v=sha) loading and the update
// check keep working exactly as without it.
self.addEventListener("install", () => self.skipWaiting());
self.addEventListener("activate", (event) => event.waitUntil(self.clients.claim()));
self.addEventListener("fetch", () => {});
