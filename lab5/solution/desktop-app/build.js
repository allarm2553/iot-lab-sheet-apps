const packager = require('electron-packager');

async function bundle() {
  console.log('Starting build...');
  try {
    const appPaths = await packager({
      dir: '.',
      name: 'MQTT-Dashboard',
      platform: 'win32',
      arch: 'x64',
      overwrite: true,
      out: 'dist'
    });
    console.log(`Successfully packaged! App paths: ${appPaths}`);
  } catch (err) {
    console.error('Packaging failed:', err);
  }
}

bundle();
