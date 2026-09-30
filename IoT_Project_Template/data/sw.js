const CACHE_NAME = 'iot-controller-pwa-v17';

const STATIC_ASSETS = [
  './',
  './index.html',
  './config.html',
  './styles.css',
  './mqtt.min.js',
  './manifest.json',
  './icon.svg',
  './icon-192.png',
  './icon-512.png',
  'https://unpkg.com/mqtt/dist/mqtt.min.js'
];

// Install Event: Pre-cache core UI assets
self.addEventListener('install', (event) => {
  self.skipWaiting();
  event.waitUntil(
    caches.open(CACHE_NAME).then((cache) => {
      console.log('[PWA SW] Pre-caching offline assets...');
      return cache.addAll(STATIC_ASSETS);
    }).catch((err) => {
      console.warn('[PWA SW] Asset caching error:', err);
    })
  );
});

// Activate Event: Cleanup stale caches & take control immediately
self.addEventListener('activate', (event) => {
  event.waitUntil(
    caches.keys().then((keys) => {
      return Promise.all(
        keys.map((key) => {
          if (key !== CACHE_NAME) {
            console.log('[PWA SW] Removing old cache:', key);
            return caches.delete(key);
          }
        })
      );
    }).then(() => self.clients.claim())
  );
});

// Fetch Event: Network-First for HTML to prevent stale dashboard cache
self.addEventListener('fetch', (event) => {
  const url = new URL(event.request.url);

  // 1. Bypass Service Worker cache for MCU REST APIs, WebSockets, or POST requests
  if (url.pathname.startsWith('/api/') || event.request.method !== 'GET') {
    return;
  }

  // 2. Network-First for HTML pages (ensures latest dashboard code is always loaded)
  const isHtml = event.request.mode === 'navigate' || (event.request.headers.get('accept') && event.request.headers.get('accept').includes('text/html'));
  if (isHtml) {
    event.respondWith(
      fetch(event.request)
        .then((networkResponse) => {
          if (networkResponse && networkResponse.status === 200) {
            caches.open(CACHE_NAME).then((cache) => cache.put(event.request, networkResponse.clone()));
          }
          return networkResponse;
        })
        .catch(() => caches.match(event.request).then((cached) => cached || caches.match('./index.html')))
    );
    return;
  }

  // 3. Cache-First for static assets (CSS, JS, icons) with Network fallback
  event.respondWith(
    caches.match(event.request).then((cachedResponse) => {
      if (cachedResponse) {
        return cachedResponse;
      }
      return fetch(event.request).then((networkResponse) => {
        if (!networkResponse || networkResponse.status !== 200) {
          return networkResponse;
        }
        const responseToCache = networkResponse.clone();
        caches.open(CACHE_NAME).then((cache) => {
          cache.put(event.request, responseToCache);
        });
        return networkResponse;
      });
    })
  );
});
