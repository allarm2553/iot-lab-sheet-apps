$zipPath = "C:\Users\terd2\AppData\Local\electron\Cache\a791fa12f2db1c58c084ec41c5caf1ac518de84788ba857a6bebef2fe9349ed3\electron-v43.1.0-win32-x64.zip"
$destPath = "c:\Users\terd2\.gemini\antigravity\scratch\sheets-webapps\iot-lab-sheet-apps\lab5\solution\MQTT-Dashboard-win32-x64"

# 1. Extract zip
Write-Host "Extracting Electron..."
Expand-Archive -Path $zipPath -DestinationPath $destPath -Force

# 2. Create resources/app
Write-Host "Configuring app package..."
$appPath = Join-Path $destPath "resources\app"
New-Item -ItemType Directory -Force -Path $appPath

# 3. Copy files to resources/app
Copy-Item -Path "c:\Users\terd2\.gemini\antigravity\scratch\sheets-webapps\iot-lab-sheet-apps\lab5\solution\data\index.html" -Destination $appPath

# 4. Create main.js for local path
$mainJsContent = @"
const { app, BrowserWindow } = require('electron');
const path = require('path');

function createWindow() {
  const win = new BrowserWindow({
    width: 1200,
    height: 850,
    title: "Cloud MQTT Real-time Dashboard",
    webPreferences: {
      nodeIntegration: false,
      contextIsolation: true
    }
  });

  win.loadFile(path.join(__dirname, 'index.html'));
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
"@
Set-Content -Path (Join-Path $appPath "main.js") -Value $mainJsContent

# 5. Create package.json inside resources/app
$packageJsonContent = @"
{
  "name": "mqtt-dashboard",
  "version": "1.0.0",
  "main": "main.js"
}
"@
Set-Content -Path (Join-Path $appPath "package.json") -Value $packageJsonContent

# 6. Rename electron.exe to MQTT-Dashboard.exe
Write-Host "Finalizing executable..."
Rename-Item -Path (Join-Path $destPath "electron.exe") -NewName "MQTT-Dashboard.exe"

Write-Host "Build complete! Output directory: $destPath"
