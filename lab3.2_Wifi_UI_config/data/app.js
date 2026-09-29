// Wi-Fi Config Portal Client Script

function scanNetworks() {
  const listEl = document.getElementById('networkList');
  listEl.innerHTML = '<div class="loading-state"><i class="fa-solid fa-spinner fa-spin"></i> กำลังค้นหาเครือข่าย Wi-Fi...</div>';

  fetch('/api/scan')
    .then(res => res.json())
    .then(networks => {
      if (!networks || networks.length === 0) {
        listEl.innerHTML = '<div class="loading-state">ไม่พบเครือข่าย Wi-Fi ในบริเวณนี้</div>';
        return;
      }
      let html = '';
      networks.forEach(net => {
        const lockIcon = net.enc ? '<i class="fa-solid fa-lock" style="font-size:0.75rem;color:#94a3b8;"></i>' : '';
        html += `
          <div class="network-item" onclick="selectSSID('${escapeHtml(net.ssid)}')">
            <div class="net-info">
              <i class="fa-solid fa-wifi" style="color:${getSignalColor(net.rssi)};"></i>
              <span>${escapeHtml(net.ssid)}</span>
              ${lockIcon}
            </div>
            <div class="net-rssi">${net.rssi} dBm</div>
          </div>
        `;
      });
      listEl.innerHTML = html;
    })
    .catch(err => {
      listEl.innerHTML = '<div class="loading-state" style="color:#f87171;"><i class="fa-solid fa-triangle-exclamation"></i> การสแกนล้มเหลว กรุณาลองใหม่อีกครั้ง</div>';
    });
}

function selectSSID(ssid) {
  document.getElementById('ssidInput').value = ssid;
  const passEl = document.getElementById('passInput');
  passEl.focus();
}

function togglePassVisibility() {
  const passEl = document.getElementById('passInput');
  const icon = document.getElementById('passToggleIcon');
  if (passEl.type === 'password') {
    passEl.type = 'text';
    icon.className = 'fa-solid fa-eye-slash';
  } else {
    passEl.type = 'password';
    icon.className = 'fa-solid fa-eye';
  }
}

function handleSave(e) {
  e.preventDefault();
  const ssid = document.getElementById('ssidInput').value.trim();
  const pass = document.getElementById('passInput').value;
  const saveBtn = document.getElementById('saveBtn');
  const alertBox = document.getElementById('statusAlert');

  if (!ssid) return;

  saveBtn.disabled = true;
  saveBtn.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> กำลังบันทึกและเชื่อมต่อ...';

  fetch('/api/save', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ ssid: ssid, pass: pass })
  })
  .then(res => res.json())
  .then(data => {
    alertBox.className = 'alert-box success';
    alertBox.style.display = 'block';
    alertBox.innerHTML = '<i class="fa-solid fa-circle-check"></i> บันทึกข้อมูลสำเร็จ! อุปกรณ์กำลังรีสตาร์ตเพื่อเชื่อมต่อเครือข่าย...';
    saveBtn.innerHTML = '<i class="fa-solid fa-check"></i> บันทึกเรียบร้อย';
  })
  .catch(err => {
    alertBox.className = 'alert-box error';
    alertBox.style.display = 'block';
    alertBox.innerHTML = '<i class="fa-solid fa-triangle-exclamation"></i> บันทึกไม่สำเร็จ: ' + err.toString();
    saveBtn.disabled = false;
    saveBtn.innerHTML = '<i class="fa-solid fa-floppy-disk"></i> บันทึกและเชื่อมต่อเครือข่าย';
  });
}

function getSignalColor(rssi) {
  if (rssi >= -60) return '#34d399';
  if (rssi >= -75) return '#fbbf24';
  return '#f87171';
}

function escapeHtml(str) {
  return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;').replace(/'/g, '&#039;');
}

window.addEventListener('DOMContentLoaded', () => {
  scanNetworks();
});
