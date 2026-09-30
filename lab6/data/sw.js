// Service Worker for Hybrid IoT Node Dashboard
const CACHE_NAME = "hybrid-pwa-v1";
const ASSETS = ["./", "./index.html", "./styles.css", "./manifest.json"];

self.addEventListener("install", (e) => {
  e.waitUntil(caches.open(CACHE_NAME).then((c) => c.addAll(ASSETS)));
  self.skipWaiting();
});

self.addEventListener("activate", (e) => {
  e.waitUntil(caches.keys().then((keys) => Promise.all(keys.filter((k) => k !== CACHE_NAME).map((k) => caches.delete(k)))));
  self.clients.claim();
});

self.addEventListener("fetch", (e) => {
  if (e.request.url.includes(":81") || e.request.url.includes("broker.emqx.io") || e.request.url.startsWith("ws")) return;
  e.respondWith(caches.match(e.request).then((res) => res || fetch(e.request)));
});
