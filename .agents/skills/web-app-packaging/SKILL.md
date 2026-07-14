---
name: Web App Packaging
description: Guidelines on packaging web-based IoT dashboards (HTML/JS/CSS) into standalone desktop applications (.exe) and mobile applications (.apk).
---

# Web App Packaging Guidelines

This skill provides step-by-step instructions to convert standard web dashboards (HTML/CSS/JS) into desktop (`.exe` on Windows) and mobile (`.apk` on Android) applications.

---

## 1. Packaging Web Apps into Desktop (.exe) via Electron

When packaging on Windows machines, standard command-line packagers (like `nativefier`) can crash or trigger security blocks. The most reliable method is to extract the Electron binary and package the files manually.

### Step 1: Create the Desktop Directory Structure
Create a directory named `desktop-app/` and add the following two configuration files.

#### `package.json`
```json
{
  "name": "iot-dashboard-app",
  "version": "1.0.0",
  "main": "main.js"
}
```

#### `main.js`
```javascript
const { app, BrowserWindow } = require('electron');
const path = require('path');

function createWindow() {
  const win = new BrowserWindow({
    width: 1200,
    height: 850,
    title: "IoT Real-time Dashboard",
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true
    }
  });

  // Load the self-contained HTML file from the same directory
  win.loadFile(path.join(__dirname, 'index.html'));
  
  // Hide the default browser menu bar
  win.setMenuBarVisibility(false);
}

app.whenReady().then(() => {
  createWindow();

  app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
      createWindow();
    }
  });
});

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit();
  }
});
```

### Step 2: Write the Manual Build Script (`build-manual.ps1`)
Use PowerShell to download/extract Electron, copy assets, and rename the executable. This bypasses PowerShell script restriction policies when executed with the bypass flag.

```powershell
# Define paths (Update electron zip directory and final output path)
$zipPath = "$env:LOCALAPPDATA\electron\Cache\<hash_directory>\electron-v<version>-win32-x64.zip"
$destPath = ".\dist\MQTT-Dashboard-win32-x64"

# 1. Extract zip
Write-Host "Extracting Electron runtime..."
Expand-Archive -Path $zipPath -DestinationPath $destPath -Force

# 2. Create resources/app
$appPath = Join-Path $destPath "resources\app"
New-Item -ItemType Directory -Force -Path $appPath

# 3. Copy files to resources/app
Copy-Item -Path ".\data\index.html" -Destination $appPath
Copy-Item -Path ".\desktop-app\main.js" -Destination $appPath
Copy-Item -Path ".\desktop-app\package.json" -Destination $appPath

# 4. Rename executable
Rename-Item -Path (Join-Path $destPath "electron.exe") -NewName "MQTT-Dashboard.exe"
Write-Host "Build complete! Output: $destPath"
```

### Step 3: Run the Script (Bypassing Execution Policy)
Run PowerShell with the bypass flag to package the app:
```powershell
powershell.exe -ExecutionPolicy Bypass -File .\build-manual.ps1
```

---

## 2. Packaging Web Apps into Mobile (.apk) for Android

To convert the web application to a mobile app, choose one of the following approaches:

### Approach A: Capacitor (Modern Hybrid Wrapper)
Capacitor runs the web app inside a native WebView container on iOS and Android.

1.  **Initialize Project:**
    ```bash
    npm install @capacitor/core @capacitor/cli
    npx cap init "IoT Dashboard" "com.example.iotdashboard" --web-dir=dist
    ```
2.  **Add Android Platform:**
    ```bash
    npm install @capacitor/android
    npx cap add android
    ```
3.  **Sync Web Assets:**
    Copy the web build (containing `index.html`) to the `dist/` directory, then sync:
    ```bash
    npx cap sync
    ```
4.  **Build APK:**
    Open the project in Android Studio and compile:
    ```bash
    npx cap open android
    ```

### Approach B: Native Android WebView wrapper (Kotlin/Java)
A lightweight alternative when building directly inside Android Studio.

1.  Create an Android project with an empty activity.
2.  Add a `WebView` component in the layout XML (`activity_main.xml`).
3.  Load the local HTML file or web server URL inside `MainActivity.kt`:
    ```kotlin
    val myWebView: WebView = findViewById(R.id.webview)
    myWebView.settings.javaScriptEnabled = true
    myWebView.loadUrl("file:///android_asset/index.html")
    ```
4.  Ensure `INTERNET` permission is added to `AndroidManifest.xml` to allow WebSocket/MQTT connections.

### Approach C: Progressive Web App (PWA)
If the web app is hosted on an HTTPS server (e.g., GitHub Pages), adding a `manifest.json` allows users to install the dashboard directly from mobile browsers.

```json
{
  "short_name": "IoT Dash",
  "name": "IoT MQTT Real-time Dashboard",
  "icons": [
    {
      "src": "icon.png",
      "type": "image/png",
      "sizes": "512x512"
    }
  ],
  "start_url": "/index.html",
  "background_color": "#0f172a",
  "display": "standalone",
  "theme_color": "#6366f1"
}
```

---

## 3. Best Practices for IoT Dashboards

*   **Secure WebSockets (WSS):** Always connect to WebSockets over WSS (port `8084` or `443` on public brokers like EMQX) instead of unencrypted WS (`8083`). Modern browsers and hybrid WebView wrappers block insecure WebSockets by default.
*   **Self-Contained HTML:** Embed CSS and JS inside the HTML page, or bundle them locally, so the packaged app can load everything offline without relying on CDN availability.
