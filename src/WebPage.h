//
// Single-page web UI served from flash. No external assets are used, so the
// phone can load it over the ESP32's own access point without internet.
//

#ifndef WEBPAGE_H
#define WEBPAGE_H

static const char INDEX_HTML[] PROGMEM = R"PAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1, viewport-fit=cover">
<meta name="theme-color" content="#070b10">
<title>Barbell Power</title>
<style>
:root{
  --bg:#070b10; --card:#101820; --card2:#0c141c; --line:#1e2c3a;
  --text:#e8f0f8; --muted:#8fa2b8; --green:#43e08a; --green2:#22c97a;
  --blue:#4cc3ff; --amber:#ffc857; --red:#ff6b6b; --shadow:0 18px 50px rgba(0,0,0,.35);
}
*{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
html,body{margin:0;padding:0;background:var(--bg);color:var(--text);font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Roboto,Helvetica,Arial,sans-serif}
body{min-height:100vh;background:
  radial-gradient(1100px 500px at 50% -220px,rgba(67,224,138,.16),transparent 70%),
  radial-gradient(800px 400px at 100% 0,rgba(76,195,255,.08),transparent 60%),
  var(--bg);padding-bottom:44px}
header{position:sticky;top:0;z-index:20;display:flex;align-items:center;justify-content:space-between;gap:12px;padding:14px 16px 10px;background:rgba(7,11,16,.86);backdrop-filter:blur(14px);border-bottom:1px solid rgba(30,44,58,.6)}
.brand{display:flex;align-items:center;gap:11px;min-width:0}
.logo{width:42px;height:42px;border-radius:13px;display:grid;place-items:center;background:linear-gradient(145deg,#173226,#0f1c18);border:1px solid #1d4734;font-size:22px;box-shadow:inset 0 1px 0 rgba(255,255,255,.06)}
.brand h1{font-size:17px;line-height:1.1;margin:0;letter-spacing:.2px}
.brand p{margin:3px 0 0;font-size:11.5px;color:var(--muted);display:flex;align-items:center;gap:6px}
.dot{width:7px;height:7px;border-radius:99px;background:var(--red);box-shadow:0 0 0 4px rgba(255,107,107,.12);flex:none}
.dot.on{background:var(--green);box-shadow:0 0 0 4px rgba(67,224,138,.12)}
.pill{font-size:11px;color:var(--muted);border:1px solid var(--line);background:var(--card2);padding:7px 10px;border-radius:99px;white-space:nowrap}
.tabs{position:sticky;top:70px;z-index:19;display:flex;gap:8px;padding:10px 16px 12px;background:linear-gradient(180deg,rgba(7,11,16,.94),rgba(7,11,16,.72),transparent)}
.tabs button{flex:1;height:42px;border-radius:12px;border:1px solid transparent;background:transparent;color:var(--muted);font-size:14px;font-weight:650;letter-spacing:.2px}
.tabs button.active{color:var(--text);background:var(--card);border-color:var(--line);box-shadow:inset 0 1px 0 rgba(255,255,255,.04)}
main{width:100%;max-width:920px;margin:0 auto;padding:0 14px}
.card{background:linear-gradient(180deg,rgba(19,29,39,.96),rgba(13,21,29,.96));border:1px solid var(--line);border-radius:18px;padding:16px;margin:12px 0;box-shadow:var(--shadow)}
.card h2{font-size:13px;letter-spacing:.8px;text-transform:uppercase;color:var(--muted);margin:0 0 12px;font-weight:750}
.row{display:flex;gap:10px;align-items:flex-end}
.row.wrap{flex-wrap:wrap}
.row.between{justify-content:space-between;align-items:center}
label.field{display:flex;flex-direction:column;gap:6px;font-size:12px;color:var(--muted);flex:1;min-width:0}
label.field .unit{color:#64748b;font-weight:500}
input,select{width:100%;height:48px;background:#0a1219;border:1px solid var(--line);border-radius:12px;color:var(--text);font-size:16px;padding:0 12px;outline:none;appearance:none}
select{background-image:linear-gradient(45deg,transparent 50%,#7c8ea3 50%),linear-gradient(135deg,#7c8ea3 50%,transparent 50%);background-position:calc(100% - 19px) 21px,calc(100% - 14px) 21px;background-size:5px 5px,5px 5px;background-repeat:no-repeat;padding-right:32px}
input:focus,select:focus{border-color:#2e5c45;box-shadow:0 0 0 3px rgba(67,224,138,.09)}
input[type=range]{height:34px;padding:0;background:transparent;border:0;box-shadow:none;accent-color:var(--green)}
input[type=checkbox]{width:22px;height:22px;accent-color:var(--green)}
button,.btn{display:inline-flex;align-items:center;justify-content:center;gap:8px;min-height:48px;padding:0 18px;border-radius:12px;border:1px solid var(--line);background:var(--card2);color:var(--text);font-size:15px;font-weight:700;text-decoration:none;cursor:pointer;transition:transform .08s ease,background .15s ease,border-color .15s ease}
button:active,.btn:active{transform:scale(.985)}
button:disabled{opacity:.4;cursor:not-allowed;transform:none}
button.primary{background:linear-gradient(180deg,var(--green),var(--green2));border-color:#2bbd74;color:#04130b;box-shadow:0 10px 28px rgba(34,201,122,.18)}
button.danger{background:linear-gradient(180deg,#ff7b7b,#e34f4f);border-color:#da4b4b;color:#1a0505}
button.ghost{background:rgba(255,255,255,.02)}
button.big{width:100%;height:56px;font-size:17px;margin-top:12px}
button.small{min-height:38px;padding:0 12px;font-size:13px;border-radius:10px}
.hint{font-size:12px;color:var(--muted);line-height:1.45;margin:10px 0 0}
.banner{max-width:920px;margin:10px auto 0;padding:11px 14px;border-radius:14px;background:rgba(255,200,87,.1);border:1px solid rgba(255,200,87,.3);color:#ffd98a;font-size:12.5px;line-height:1.45}
.banner.bad{background:rgba(255,107,107,.09);border-color:rgba(255,107,107,.32);color:#ffb1b1}
.hidden{display:none !important}
.hero{padding:18px 16px 16px;text-align:center;position:relative;overflow:hidden}
.hero:before{content:"";position:absolute;inset:-40% -20% auto;height:230px;background:radial-gradient(circle,rgba(67,224,138,.15),transparent 66%);pointer-events:none}
.power{font-variant-numeric:tabular-nums;font-size:72px;line-height:.95;font-weight:820;letter-spacing:-3px;color:#f4fff9;text-shadow:0 0 42px rgba(67,224,138,.2);position:relative}
.power .unit{font-size:22px;font-weight:700;letter-spacing:0;color:var(--muted);margin-left:6px;vertical-align:6px}
.heroSub{display:flex;align-items:center;justify-content:center;gap:12px;margin-top:11px;color:var(--muted);font-size:12.5px}
.phase{display:inline-flex;align-items:center;gap:6px;padding:5px 9px;border-radius:99px;border:1px solid var(--line);background:rgba(255,255,255,.02);font-weight:700;font-size:11px;letter-spacing:.5px;text-transform:uppercase}
.phase.con{color:var(--green);border-color:rgba(67,224,138,.35);background:rgba(67,224,138,.08)}
.phase.ecc{color:var(--blue);border-color:rgba(76,195,255,.32);background:rgba(76,195,255,.07)}
.grid3{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:18px}
.grid3 > div{background:rgba(0,0,0,.18);border:1px solid rgba(30,44,58,.8);border-radius:14px;padding:11px 6px}
.grid3 b{display:block;font-size:22px;font-variant-numeric:tabular-nums;line-height:1.1}
.grid3 small{display:block;color:var(--muted);font-size:10px;text-transform:uppercase;letter-spacing:.7px;margin-top:4px}
.stats{display:grid;grid-template-columns:repeat(5,1fr);gap:8px;text-align:center;margin-top:0}
.stats > div{padding:4px 2px}
.stats b{display:block;font-size:17px;font-variant-numeric:tabular-nums}
.stats small{display:block;color:var(--muted);font-size:9.5px;text-transform:uppercase;letter-spacing:.5px;margin-top:2px}
.chartCard{padding:10px 10px 6px}
#chart{display:block;width:100%;height:150px;border-radius:12px;background:linear-gradient(180deg,rgba(76,195,255,.03),rgba(0,0,0,.12))}
table{width:100%;border-collapse:collapse;font-variant-numeric:tabular-nums}
th{font-size:10px;text-transform:uppercase;letter-spacing:.55px;color:var(--muted);font-weight:650;text-align:right;padding:0 4px 8px}
th:first-child,td:first-child{text-align:left}
td{font-size:12.5px;text-align:right;padding:8px 4px;border-top:1px solid rgba(30,44,58,.62)}
tr:first-child td{border-top:0}
.muted{color:var(--muted)}
.tableWrap{overflow-x:auto}
.switch{display:flex;align-items:center;gap:12px;min-height:48px;font-size:14px;color:var(--text)}
.switch input{flex:none}
.sliderRow{display:flex;justify-content:space-between;align-items:center;margin-top:14px;font-size:12px;color:var(--muted)}
.sliderRow b{color:var(--text)}
.seg{display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:12px}
.seg button{height:44px;min-height:44px;font-size:13px}
.seg button.on{border-color:rgba(67,224,138,.5);background:rgba(67,224,138,.1);color:var(--green)}
.calibReadout{display:flex;align-items:baseline;gap:8px;background:#0a1219;border:1px solid var(--line);border-radius:14px;padding:14px;margin:12px 0}
.calibReadout b{font-size:28px;font-variant-numeric:tabular-nums}
.calibReadout span{color:var(--muted);font-size:12px}
.sessionItem{border:1px solid var(--line);border-radius:14px;padding:13px;margin-top:10px;background:rgba(0,0,0,.16)}
.sessionItem .top{display:flex;justify-content:space-between;gap:10px;align-items:flex-start}
.sessionItem h3{margin:0;font-size:15px}
.sessionItem .date{font-size:11px;color:var(--muted);margin-top:3px}
.sessionItem .badge{font-size:10px;padding:4px 7px;border-radius:99px;background:rgba(255,200,87,.1);border:1px solid rgba(255,200,87,.3);color:#ffd98a;white-space:nowrap}
.sessionStats{display:grid;grid-template-columns:repeat(4,1fr);gap:8px;margin:12px 0}
.sessionStats div{background:rgba(255,255,255,.02);border-radius:10px;padding:8px 6px;text-align:center}
.sessionStats b{display:block;font-size:15px;font-variant-numeric:tabular-nums}
.sessionStats small{display:block;font-size:9px;color:var(--muted);text-transform:uppercase;letter-spacing:.4px;margin-top:2px}
.storage{margin:12px 0}
.bar{height:8px;background:#0a1219;border:1px solid var(--line);border-radius:99px;overflow:hidden}
.bar > div{height:100%;width:0;background:linear-gradient(90deg,var(--green),var(--blue));transition:width .3s ease}
.storageText{display:flex;justify-content:space-between;font-size:11px;color:var(--muted);margin-top:6px}
.actions{display:flex;gap:8px;flex-wrap:wrap;margin-top:8px}
.toast{position:fixed;left:50%;bottom:22px;transform:translate(-50%,20px);background:#10231a;color:#d9ffe9;border:1px solid #28543d;padding:12px 16px;border-radius:99px;font-size:13px;font-weight:650;opacity:0;pointer-events:none;transition:.22s ease;z-index:50;max-width:88vw;text-align:center;box-shadow:var(--shadow)}
.toast.show{opacity:1;transform:translate(-50%,0)}
.toast.err{background:#2a1113;color:#ffc9c9;border-color:#5c262a}
.summaryGrid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin:12px 0}
.summaryGrid > div{background:rgba(255,255,255,.02);border:1px solid rgba(30,44,58,.7);border-radius:12px;padding:10px;text-align:center}
.summaryGrid b{display:block;font-size:19px;font-variant-numeric:tabular-nums}
.summaryGrid small{display:block;font-size:9px;color:var(--muted);text-transform:uppercase;letter-spacing:.4px;margin-top:2px}
.deviceLine{display:flex;justify-content:space-between;gap:12px;font-size:13px;padding:9px 0;border-bottom:1px solid rgba(30,44,58,.55)}
.deviceLine:last-of-type{border-bottom:0}
.deviceLine span{color:var(--muted)}
@media (max-width:430px){
  .power{font-size:62px}
  .stats{grid-template-columns:repeat(3,1fr);row-gap:13px}
  .stats > div:nth-child(4),.stats > div:nth-child(5){grid-column:span 1}
  .sessionStats{grid-template-columns:repeat(2,1fr)}
  .tabs{top:68px}
  .row{flex-direction:column;align-items:stretch}
  .row .btn,.row button{flex:1}
}
</style>
</head>
<body>
<header>
  <div class="brand">
    <div class="logo">⚡</div>
    <div>
      <h1>Barbell Power</h1>
      <p><span id="dot" class="dot"></span><span id="conn">Connecting…</span></p>
    </div>
  </div>
  <div class="pill" id="ipPill">192.168.4.1</div>
</header>

<div id="banner" class="banner hidden"></div>

<nav class="tabs">
  <button data-tab="live" class="active">Live</button>
  <button data-tab="history">History</button>
  <button data-tab="settings">Settings</button>
</nav>

<main>
  <!-- LIVE -->
  <section id="tab-live">
    <div class="card">
      <div class="row">
        <label class="field">Total weight <span class="unit">kg</span>
          <input id="weight" type="number" inputmode="decimal" min="0.5" max="1000" step="0.5" value="60">
        </label>
        <label class="field">Exercise
          <select id="exercise">
            <option>Squat</option>
            <option>Bench Press</option>
            <option>Deadlift</option>
            <option>Overhead Press</option>
            <option>Barbell Row</option>
            <option>Romanian Deadlift</option>
            <option>Hip Thrust</option>
            <option>Custom</option>
          </select>
        </label>
      </div>
      <input id="customExercise" class="hidden" style="margin-top:10px" maxlength="40" placeholder="Custom exercise name">
      <button id="sessionBtn" class="primary big">Start set</button>
      <p class="hint">Enter the total moving weight (bar + plates). Only the lifting phase produces watts.</p>
    </div>

    <div class="card hero">
      <div class="power"><span id="power">0</span><span class="unit">W</span></div>
      <div class="heroSub">
        <span id="phase" class="phase">Idle</span>
        <span id="elapsed">0:00</span>
      </div>
      <div class="grid3">
        <div><b id="velocity">0.00</b><small>m/s</small></div>
        <div><b id="position">0</b><small>cm</small></div>
        <div><b id="reps">0</b><small>reps</small></div>
      </div>
    </div>

    <div class="card chartCard"><canvas id="chart"></canvas></div>

    <div class="card">
      <div class="stats">
        <div><b id="avgPower">0</b><small>avg W</small></div>
        <div><b id="peakPower">0</b><small>peak W</small></div>
        <div><b id="rom">0</b><small>ROM cm</small></div>
        <div><b id="tut">0.0</b><small>TUT s</small></div>
        <div><b id="work">0</b><small>joules</small></div>
      </div>
    </div>

    <div class="card">
      <h2>Reps</h2>
      <div class="tableWrap">
        <table>
          <thead><tr><th>#</th><th>ROM cm</th><th>Avg W</th><th>Peak W</th><th>Avg m/s</th><th>Time</th></tr></thead>
          <tbody id="repBody"><tr><td colspan="6" class="muted">No reps yet</td></tr></tbody>
        </table>
      </div>
    </div>

    <div class="card hidden" id="summaryCard">
      <h2>Set complete</h2>
      <div id="summaryTitle" class="hint" style="margin-top:0"></div>
      <div class="summaryGrid">
        <div><b id="sumAvg">0</b><small>avg W</small></div>
        <div><b id="sumPeak">0</b><small>peak W</small></div>
        <div><b id="sumReps">0</b><small>reps</small></div>
        <div><b id="sumVel">0.00</b><small>avg m/s</small></div>
        <div><b id="sumRom">0</b><small>ROM cm</small></div>
        <div><b id="sumWork">0</b><small>joules</small></div>
      </div>
      <a id="summaryDownload" class="btn primary" href="#">Download raw CSV</a>
      <a id="summaryMeta" class="btn ghost small hidden" href="#" style="margin-top:8px">Session metadata (JSON)</a>
    </div>
  </section>

  <!-- HISTORY -->
  <section id="tab-history" class="hidden">
    <div class="card">
      <div class="row between">
        <h2 style="margin:0">Sessions</h2>
        <button id="refreshHistory" class="ghost small">Refresh</button>
      </div>
      <div class="storage">
        <div class="bar"><div id="storageBar"></div></div>
        <div class="storageText"><span id="storageText">Storage</span><span id="storagePct">0%</span></div>
      </div>
      <div class="actions">
        <a class="btn ghost small" href="/api/export/summary.csv">Download summary CSV</a>
        <button id="deleteAll" class="ghost small" style="color:var(--red)">Delete all</button>
      </div>
      <div id="sessionList"><p class="hint">No sessions yet.</p></div>
    </div>
  </section>

  <!-- SETTINGS -->
  <section id="tab-settings" class="hidden">
    <div class="card">
      <h2>1 m calibration</h2>
      <p class="hint" style="margin-top:0">Pull the string out <b>exactly 1.00 m</b> in the direction it moves when you lower the bar, then press Finish. The firmware also learns which encoder direction is “up”.</p>
      <div class="calibReadout"><b id="calCounts">0</b><span id="calCm">counts</span></div>
      <div class="actions">
        <button id="calStart" class="primary">Start calibration</button>
        <button id="calFinish" class="ghost" disabled>Finish</button>
        <button id="calCancel" class="ghost" disabled>Cancel</button>
      </div>
      <p class="hint" id="calInfo">Not calibrated yet.</p>
    </div>

    <div class="card">
      <h2>String take-up</h2>
      <label class="switch"><input type="checkbox" id="takeupEnabled"><span>Keep a light pull on the string</span></label>
      <div class="sliderRow"><span>Strength</span><b id="takeupVal">15%</b></div>
      <input type="range" id="takeupPercent" min="0" max="100" step="1" value="15">
      <div class="seg">
        <button id="dirA" data-dir="0">Direction A</button>
        <button id="dirB" data-dir="1">Direction B</button>
      </div>
      <button id="autoDir" class="ghost" style="width:100%;margin-top:10px">Auto-detect take-up direction</button>
      <p class="hint">The motor pulls gently so the string spools back in. Keep the strength just high enough to remove slack. Auto-detect runs the motor for half a second.</p>
    </div>

    <div class="card">
      <h2>Rep detection</h2>
      <div class="sliderRow"><span>Minimum rep range of motion</span><b id="minRomVal">10 cm</b></div>
      <input type="range" id="minRom" min="2" max="60" step="1" value="10">
      <div class="sliderRow"><span>Velocity deadband</span><b id="deadbandVal">0.03 m/s</b></div>
      <input type="range" id="deadband" min="0" max="0.2" step="0.01" value="0.03">
      <div class="actions">
        <button id="zeroBtn" class="ghost">Zero position now</button>
        <button id="flipBtn" class="ghost">Flip lift direction</button>
      </div>
      <p class="hint">Increase the deadband if the take-up motor creates phantom reps. Minimum ROM filters out small setup movements.</p>
    </div>

    <div class="card">
      <h2>Device</h2>
      <div class="deviceLine"><span>WiFi network</span><b id="ssid">BarbellPower</b></div>
      <div class="deviceLine"><span>Website</span><b>http://192.168.4.1</b></div>
      <div class="deviceLine"><span>Uptime</span><b id="uptime">—</b></div>
      <div class="deviceLine"><span>Free memory</span><b id="heap">—</b></div>
      <div class="deviceLine"><span>Lift direction</span><b id="dirInfo">—</b></div>
      <button id="rebootBtn" class="ghost" style="width:100%;margin-top:12px;color:var(--red)">Reboot ESP32</button>
    </div>
  </section>
</main>

<div id="toast" class="toast"></div>

<script>
(function(){
  "use strict";
  const $ = (id) => document.getElementById(id);
  const state = {
    data: null,
    active: false,
    calibrating: false,
    takeupDir: false,
    dirSaving: false,
    settingsLoaded: false,
    polling: false,
    chart: [],
    repSig: "",
    summarySig: "",
    lastTab: "live",
    saveTimer: null
  };

  function esc(value){
    return String(value == null ? "" : value).replace(/[&<>"']/g, function(c){
      return {"&":"&amp;","<":"&lt;",">":"&gt;",'"':"&quot;","'":"&#39;"}[c];
    });
  }
  function num(value, digits){
    const n = Number(value);
    return (isFinite(n) ? n : 0).toFixed(digits == null ? 0 : digits);
  }
  function fmtTime(ms){
    const total = Math.max(0, Math.floor((ms || 0) / 1000));
    const m = Math.floor(total / 60);
    const s = total % 60;
    return m + ":" + (s < 10 ? "0" : "") + s;
  }
  function fmtDate(epoch){
    if (!epoch) return "Unknown date";
    const d = new Date(epoch * 1000);
    if (isNaN(d.getTime())) return "Unknown date";
    return d.toLocaleDateString(undefined, {year:"numeric", month:"short", day:"numeric"}) +
      " · " + d.toLocaleTimeString(undefined, {hour:"2-digit", minute:"2-digit"});
  }
  function fmtUptime(ms){
    const s = Math.floor((ms || 0) / 1000);
    const d = Math.floor(s / 86400);
    const h = Math.floor((s % 86400) / 3600);
    const m = Math.floor((s % 3600) / 60);
    if (d > 0) return d + "d " + h + "h";
    if (h > 0) return h + "h " + m + "m";
    return m + "m " + (s % 60) + "s";
  }
  function toast(message, isError){
    const el = $("toast");
    el.textContent = message;
    el.className = "toast show" + (isError ? " err" : "");
    clearTimeout(toast._timer);
    toast._timer = setTimeout(function(){ el.className = "toast"; }, 2600);
  }
  function setConnection(online){
    $("dot").className = "dot" + (online ? " on" : "");
    $("conn").textContent = online ? "Online" : "Offline — check WiFi";
  }
  function showBanner(message, bad){
    const el = $("banner");
    if (!message){ el.className = "banner hidden"; return; }
    el.textContent = message;
    el.className = "banner" + (bad ? " bad" : "");
  }

  async function api(path, options){
    const res = await fetch(path, Object.assign({cache:"no-store"}, options || {}));
    const text = await res.text();
    let data = {};
    if (text) {
      try { data = JSON.parse(text); }
      catch (e) { throw new Error(text.slice(0, 160) || ("HTTP " + res.status)); }
    }
    if (!res.ok) throw new Error(data.error || data.message || ("HTTP " + res.status));
    return data;
  }
  function postForm(path, fields){
    const body = new URLSearchParams();
    Object.keys(fields || {}).forEach(function(key){
      if (fields[key] !== undefined && fields[key] !== null) body.append(key, fields[key]);
    });
    return api(path, {
      method:"POST",
      headers:{"Content-Type":"application/x-www-form-urlencoded"},
      body:body
    });
  }

  function updateRepTable(reps){
    const body = $("repBody");
    if (!reps || reps.length === 0){
      body.innerHTML = '<tr><td colspan="6" class="muted">No reps yet</td></tr>';
      return;
    }
    const rows = reps.slice().reverse().map(function(r){
      return "<tr><td>" + r.i + "</td><td>" + num(r.romCm, 1) + "</td><td>" +
        num(r.avgW) + "</td><td>" + num(r.peakW) + "</td><td>" +
        num(r.avgV, 2) + "</td><td>" + num((r.ms || 0) / 1000, 2) + "s</td></tr>";
    }).join("");
    body.innerHTML = rows;
  }

  function drawChart(){
    const canvas = $("chart");
    const rect = canvas.getBoundingClientRect();
    if (rect.width < 10) return;
    const dpr = window.devicePixelRatio || 1;
    const w = rect.width;
    const h = rect.height;
    if (canvas.width !== Math.floor(w * dpr) || canvas.height !== Math.floor(h * dpr)){
      canvas.width = Math.floor(w * dpr);
      canvas.height = Math.floor(h * dpr);
    }
    const ctx = canvas.getContext("2d");
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.clearRect(0, 0, w, h);

    ctx.strokeStyle = "rgba(143,162,184,.12)";
    ctx.lineWidth = 1;
    for (let i = 1; i <= 3; i++){
      const y = (h - 16) * i / 4 + 8;
      ctx.beginPath(); ctx.moveTo(0, y); ctx.lineTo(w, y); ctx.stroke();
    }

    const data = state.chart;
    if (data.length < 2){
      ctx.fillStyle = "rgba(143,162,184,.62)";
      ctx.font = "12px -apple-system, Segoe UI, Roboto, sans-serif";
      ctx.textAlign = "center";
      ctx.fillText(state.active ? "Waiting for movement…" : "Start a set to see power", w / 2, h / 2);
      return;
    }

    let max = 100;
    for (let i = 0; i < data.length; i++) if (data[i] > max) max = data[i];
    max *= 1.12;

    const padX = 2;
    const top = 8;
    const bottom = h - 8;
    function xAt(i){ return padX + (w - padX * 2) * i / (data.length - 1); }
    function yAt(v){ return bottom - (bottom - top) * (v / max); }

    const gradient = ctx.createLinearGradient(0, top, 0, bottom);
    gradient.addColorStop(0, "rgba(67,224,138,.34)");
    gradient.addColorStop(1, "rgba(67,224,138,.01)");
    ctx.beginPath();
    ctx.moveTo(xAt(0), bottom);
    for (let i = 0; i < data.length; i++) ctx.lineTo(xAt(i), yAt(data[i]));
    ctx.lineTo(xAt(data.length - 1), bottom);
    ctx.closePath();
    ctx.fillStyle = gradient;
    ctx.fill();

    ctx.beginPath();
    for (let i = 0; i < data.length; i++){
      const x = xAt(i), y = yAt(data[i]);
      if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y);
    }
    ctx.strokeStyle = "#43e08a";
    ctx.lineWidth = 2;
    ctx.lineJoin = "round";
    ctx.stroke();

    const lastX = xAt(data.length - 1);
    const lastY = yAt(data[data.length - 1]);
    ctx.beginPath(); ctx.arc(lastX, lastY, 3.5, 0, Math.PI * 2);
    ctx.fillStyle = "#b9ffd8"; ctx.fill();

    ctx.fillStyle = "rgba(143,162,184,.7)";
    ctx.font = "10px -apple-system, Segoe UI, Roboto, sans-serif";
    ctx.textAlign = "left";
    ctx.fillText(Math.round(max) + " W", 6, 14);
  }

  function renderSummary(summary, active){
    const card = $("summaryCard");
    if (!summary || !summary.valid || active){ card.classList.add("hidden"); return; }
    const sig = summary.id + "|" + summary.reps + "|" + summary.avgPowerW + "|" + summary.peakPowerW;
    if (sig === state.summarySig) { card.classList.remove("hidden"); return; }
    state.summarySig = sig;
    $("summaryTitle").innerHTML = esc(summary.exercise) + " · " + num(summary.weightKg, 1) + " kg · " +
      esc(fmtDate(summary.epoch)) + " · " + num((summary.durationMs || 0) / 1000, 1) + " s";
    $("sumAvg").textContent = num(summary.avgPowerW);
    $("sumPeak").textContent = num(summary.peakPowerW);
    $("sumReps").textContent = num(summary.reps);
    $("sumVel").textContent = num(summary.avgVelocityMps, 2);
    $("sumRom").textContent = num(summary.romCm, 1);
    $("sumWork").textContent = num(summary.workJ);
    const link = $("summaryDownload");
    const meta = $("summaryMeta");
    if (summary.id && summary.id !== "live"){
      link.href = "/api/sessions/download?id=" + encodeURIComponent(summary.id);
      link.classList.remove("hidden");
      meta.href = "/api/sessions/metadata?id=" + encodeURIComponent(summary.id);
      meta.classList.remove("hidden");
    } else {
      link.classList.add("hidden");
      meta.classList.add("hidden");
    }
    card.classList.remove("hidden");
  }

  function render(d){
    state.data = d;
    state.active = !!(d.session && d.session.active);
    state.calibrating = !!(d.calibration && d.calibration.active);

    const live = d.live || {};
    const session = d.session || {};
    const cal = d.calibration || {};
    const settings = d.settings || {};

    setConnection(true);
    $("ipPill").textContent = (d.device && d.device.ip) || "192.168.4.1";

    let banner = "";
    let bad = false;
    if (!cal.calibrated){
      banner = "Not calibrated yet. Open Settings and do the 1 m pull before training.";
      bad = false;
    }
    if (d.storage && !d.storage.available){
      banner = "Storage is unavailable — this set will not be saved. Check the LittleFS partition.";
      bad = true;
    } else if (state.active && session.recording === false){
      banner = "This set is running without recording.";
      bad = true;
    }
    showBanner(banner, bad);

    const phaseNames = ["Idle", "Eccentric", "Concentric"];
    $("phase").textContent = phaseNames[state.active ? (session.phase || 0) : 0] || "Idle";
    $("phase").className = "phase" + (!state.active ? "" : (session.phase === 2 ? " con" : (session.phase === 1 ? " ecc" : "")));

    $("power").textContent = Math.round(state.active ? (session.power || 0) : 0);
    $("velocity").textContent = num(live.velocity, 2);
    $("position").textContent = num((live.positionM || 0) * 100, 0);
    $("reps").textContent = num(state.active ? (session.reps || 0) : 0);
    $("elapsed").textContent = fmtTime(state.active ? session.elapsedMs : 0);

    $("avgPower").textContent = num(session.avgPower);
    $("peakPower").textContent = num(session.peakPower);
    $("rom").textContent = num(session.romCm, 1);
    $("tut").textContent = num((session.tutMs || 0) / 1000, 1);
    $("work").textContent = num(session.workJ);

    const reps = d.lastReps || [];
    const repSig = reps.map(function(r){ return r.i + ":" + r.peakW; }).join("|");
    if (repSig !== state.repSig){ state.repSig = repSig; updateRepTable(reps); }

    renderSummary(d.lastSummary, state.active);

    if (state.active){
      state.chart.push(Math.max(0, session.power || 0));
      if (state.chart.length > 120) state.chart.shift();
    }
    drawChart();

    const btn = $("sessionBtn");
    if (state.calibrating){
      btn.textContent = "Calibrating…";
      btn.className = "primary big";
      btn.disabled = true;
    } else {
      btn.disabled = false;
      btn.textContent = state.active ? "Stop set" : "Start set";
      btn.className = (state.active ? "danger" : "primary") + " big";
    }
    $("weight").disabled = state.active || state.calibrating;
    $("exercise").disabled = state.active || state.calibrating;
    $("customExercise").disabled = state.active || state.calibrating;

    // Calibration UI
    const counts = cal.counts || 0;
    $("calCounts").textContent = Math.abs(counts);
    if (cal.calibrated && cal.countsPerMeter > 0){
      $("calCm").textContent = "counts · ≈ " + num(Math.abs(counts) / cal.countsPerMeter * 100, 1) + " cm";
    } else {
      $("calCm").textContent = "counts";
    }
    $("calInfo").textContent = cal.calibrated
      ? ("Calibrated: " + num(cal.countsPerMeter, 1) + " counts/m. Lift direction is " + (cal.extensionSign >= 0 ? "A" : "B") + ".")
      : "Not calibrated yet.";
    $("calStart").disabled = state.calibrating || state.active;
    $("calFinish").disabled = !state.calibrating;
    $("calCancel").disabled = !state.calibrating;

    // Settings controls (do not fight a control the user is dragging).
    if (!state.dirSaving) state.takeupDir = !!settings.takeupDir;
    if (!state.settingsLoaded){
      state.settingsLoaded = true;
      $("weight").value = settings.lastWeightKg || 60;
      const ex = settings.lastExercise || "Squat";
      const select = $("exercise");
      let found = false;
      for (let i = 0; i < select.options.length; i++){
        if (select.options[i].value === ex){ select.selectedIndex = i; found = true; break; }
      }
      if (!found){ select.value = "Custom"; $("customExercise").value = ex; $("customExercise").classList.remove("hidden"); }
    }
    if (document.activeElement !== $("takeupPercent")) $("takeupPercent").value = settings.takeupPercent;
    if (document.activeElement !== $("minRom")) $("minRom").value = settings.minRepRomCm;
    if (document.activeElement !== $("deadband")) $("deadband").value = settings.velocityDeadband;
    if (document.activeElement !== $("takeupEnabled")) $("takeupEnabled").checked = !!settings.takeupEnabled;
    $("takeupVal").textContent = num(settings.takeupPercent) + "%";
    $("minRomVal").textContent = num(settings.minRepRomCm, 0) + " cm";
    $("deadbandVal").textContent = num(settings.velocityDeadband, 2) + " m/s";
    $("dirA").className = state.takeupDir ? "" : "on";
    $("dirB").className = state.takeupDir ? "on" : "";

    // Device + storage
    const device = d.device || {};
    const storage = d.storage || {};
    $("ssid").textContent = device.ssid || "BarbellPower";
    $("uptime").textContent = fmtUptime(device.uptimeMs);
    $("heap").textContent = num((device.freeHeap || 0) / 1024, 0) + " KB";
    $("dirInfo").textContent = cal.extensionSign >= 0 ? "A (lift)" : "B (lift)";
    updateStorage(storage);
  }

  function updateStorage(storage){
    if (!storage || !storage.total) { $("storageText").textContent = "Storage unavailable"; $("storagePct").textContent = ""; $("storageBar").style.width = "0%"; return; }
    const pct = Math.max(0, Math.min(100, (storage.used / storage.total) * 100));
    $("storageBar").style.width = pct.toFixed(1) + "%";
    $("storageText").textContent = num(storage.used / 1048576, 2) + " / " + num(storage.total / 1048576, 2) + " MB used";
    $("storagePct").textContent = pct.toFixed(0) + "%";
  }

  async function poll(){
    if (state.polling) return;
    state.polling = true;
    try {
      const d = await api("/api/status");
      render(d);
    } catch (e) {
      setConnection(false);
    } finally {
      state.polling = false;
    }
  }

  async function toggleSession(){
    if (state.calibrating) { toast("Finish or cancel calibration first.", true); return; }
    const btn = $("sessionBtn");
    btn.disabled = true;
    try {
      if (state.active){
        await postForm("/api/session/stop", {});
        toast("Set saved");
        state.summarySig = "";
        loadHistory();
      } else {
        const weight = parseFloat($("weight").value);
        if (!(weight > 0)) { toast("Enter a valid weight.", true); return; }
        let exercise = $("exercise").value;
        if (exercise === "Custom"){
          exercise = $("customExercise").value.trim();
          if (!exercise) exercise = "Custom";
        }
        if (state.data && state.data.calibration && !state.data.calibration.calibrated){
          if (!confirm("The string is not calibrated yet. Start the set anyway?")) return;
        }
        state.chart = [];
        state.repSig = "";
        state.summarySig = "";
        $("summaryCard").classList.add("hidden");
        await postForm("/api/session/start", {
          weight: weight,
          exercise: exercise,
          epoch: Math.floor(Date.now() / 1000)
        });
        toast("Set started — lift!");
      }
      await poll();
    } catch (e) {
      toast(e.message || "Request failed", true);
    } finally {
      btn.disabled = false;
    }
  }

  function scheduleSave(extra){
    clearTimeout(state.saveTimer);
    state.saveTimer = setTimeout(async function(){
      const fields = {
        takeupEnabled: $("takeupEnabled").checked ? 1 : 0,
        takeupPercent: $("takeupPercent").value,
        minRepRomCm: $("minRom").value,
        velocityDeadband: $("deadband").value
      };
      Object.keys(extra || {}).forEach(function(k){ fields[k] = extra[k]; });
      try {
        await postForm("/api/settings", fields);
      } catch (e) {
        toast(e.message || "Could not save settings", true);
      }
    }, 220);
  }

  async function loadHistory(){
    const list = $("sessionList");
    list.innerHTML = '<p class="hint">Loading…</p>';
    try {
      const sessions = await api("/api/sessions");
      if (!sessions.length){
        list.innerHTML = '<p class="hint">No sessions yet. Start a set from the Live tab.</p>';
        return;
      }
      sessions.sort(function(a, b){ return (b.epoch || 0) - (a.epoch || 0); });
      const activeId = (state.data && state.data.session && state.data.session.active) ? state.data.session.id : "";
      list.innerHTML = sessions.map(function(s){
        const isActive = activeId && s.id === activeId;
        let badge = "";
        if (isActive) badge = '<span class="badge">Recording</span>';
        else if (!s.finished) badge = '<span class="badge">Incomplete</span>';
        const actions = isActive ? '' :
          '<div class="actions">' +
            '<a class="btn ghost small" href="/api/sessions/download?id=' + encodeURIComponent(s.id) + '">Raw CSV</a>' +
            '<a class="btn ghost small" href="/api/sessions/metadata?id=' + encodeURIComponent(s.id) + '">JSON</a>' +
            '<button class="ghost small" data-del="' + esc(s.id) + '" style="color:var(--red)">Delete</button>' +
          '</div>';
        return '<div class="sessionItem">' +
          '<div class="top"><div><h3>' + esc(s.exercise) + ' · ' + num(s.weightKg, 1) + ' kg</h3>' +
          '<div class="date">' + esc(fmtDate(s.epoch)) + '</div></div>' +
          badge + '</div>' +
          '<div class="sessionStats">' +
            '<div><b>' + num(s.reps) + '</b><small>reps</small></div>' +
            '<div><b>' + num(s.avgPowerW) + '</b><small>avg W</small></div>' +
            '<div><b>' + num(s.peakPowerW) + '</b><small>peak W</small></div>' +
            '<div><b>' + num((s.durationS || 0), 0) + '</b><small>seconds</small></div>' +
          '</div>' + actions + '</div>';
      }).join("");
    } catch (e) {
      list.innerHTML = '<p class="hint">Could not load sessions: ' + esc(e.message) + '</p>';
    }
  }

  function switchTab(name){
    state.lastTab = name;
    document.querySelectorAll(".tabs button").forEach(function(btn){
      btn.classList.toggle("active", btn.dataset.tab === name);
    });
    ["live", "history", "settings"].forEach(function(tab){
      $("tab-" + tab).classList.toggle("hidden", tab !== name);
    });
    if (name === "history"){
      if (state.data && state.data.storage) updateStorage(state.data.storage);
      loadHistory();
    }
    window.scrollTo({top:0, behavior:"smooth"});
  }

  // ----- events -----
  document.querySelectorAll(".tabs button").forEach(function(btn){
    btn.addEventListener("click", function(){ switchTab(btn.dataset.tab); });
  });

  $("sessionBtn").addEventListener("click", toggleSession);

  $("exercise").addEventListener("change", function(){
    $("customExercise").classList.toggle("hidden", this.value !== "Custom");
    if (this.value === "Custom") $("customExercise").focus();
  });

  ["takeupEnabled", "takeupPercent", "minRom", "deadband"].forEach(function(id){
    const el = $(id);
    const handler = function(){
      if (id === "takeupPercent") $("takeupVal").textContent = el.value + "%";
      if (id === "minRom") $("minRomVal").textContent = el.value + " cm";
      if (id === "deadband") $("deadbandVal").textContent = Number(el.value).toFixed(2) + " m/s";
      scheduleSave();
    };
    el.addEventListener("input", handler);
    el.addEventListener("change", handler);
  });

  async function setTakeupDir(dir){
    state.takeupDir = dir;
    state.dirSaving = true;
    $("dirA").className = dir ? "" : "on";
    $("dirB").className = dir ? "on" : "";
    try {
      await postForm("/api/settings", {takeupDir: dir ? 1 : 0});
      toast("Take-up direction " + (dir ? "B" : "A") + " saved");
    } catch (e) {
      toast(e.message || "Could not save direction", true);
    } finally {
      state.dirSaving = false;
      poll();
    }
  }
  $("dirA").addEventListener("click", function(){ setTakeupDir(false); });
  $("dirB").addEventListener("click", function(){ setTakeupDir(true); });

  $("autoDir").addEventListener("click", async function(){
    if (!confirm("The motor will run for about half a second. Make sure the string can move freely and the bar is not attached. Continue?")) return;
    this.disabled = true;
    try {
      const r = await postForm("/api/takeup/autodir", {});
      toast(r.message || "Direction detected");
      await poll();
    } catch (e) {
      toast(e.message || "Auto-detect failed", true);
    } finally {
      this.disabled = false;
    }
  });

  $("calStart").addEventListener("click", async function(){
    try {
      await postForm("/api/calibrate/start", {});
      toast("Pull the string out exactly 1.00 m");
      await poll();
    } catch (e) { toast(e.message || "Could not start calibration", true); }
  });
  $("calFinish").addEventListener("click", async function(){
    try {
      const r = await postForm("/api/calibrate/finish", {});
      toast(r.message || "Calibrated");
      await poll();
    } catch (e) { toast(e.message || "Calibration failed", true); }
  });
  $("calCancel").addEventListener("click", async function(){
    try { await postForm("/api/calibrate/cancel", {}); toast("Calibration cancelled"); await poll(); }
    catch (e) { toast(e.message || "Could not cancel", true); }
  });

  $("zeroBtn").addEventListener("click", async function(){
    if (!confirm("Set the current string position as zero? If a set is running, its rep statistics restart from here.")) return;
    try { await postForm("/api/zero", {}); toast("Position zeroed"); await poll(); }
    catch (e) { toast(e.message || "Could not zero", true); }
  });

  $("flipBtn").addEventListener("click", async function(){
    if (!confirm("Flip the lift direction? Do this if lifting shows as negative velocity, then re-run auto-detect for take-up.")) return;
    try { await postForm("/api/flip-direction", {}); toast("Lift direction flipped"); await poll(); }
    catch (e) { toast(e.message || "Could not flip", true); }
  });

  $("refreshHistory").addEventListener("click", loadHistory);

  $("sessionList").addEventListener("click", async function(event){
    const id = event.target && event.target.getAttribute("data-del");
    if (!id) return;
    if (!confirm("Delete this session?")) return;
    try {
      await postForm("/api/sessions/delete", {id: id});
      toast("Session deleted");
      loadHistory();
      poll();
    } catch (e) { toast(e.message || "Delete failed", true); }
  });

  $("deleteAll").addEventListener("click", async function(){
    if (!confirm("Delete every saved session? This cannot be undone.")) return;
    try {
      await postForm("/api/sessions/deleteAll", {});
      toast("All sessions deleted");
      loadHistory();
      poll();
    } catch (e) { toast(e.message || "Delete failed", true); }
  });

  $("rebootBtn").addEventListener("click", async function(){
    if (!confirm("Reboot the ESP32?")) return;
    try {
      await postForm("/api/reboot", {});
      toast("Rebooting…");
      setConnection(false);
    } catch (e) { toast(e.message || "Reboot failed", true); }
  });

  window.addEventListener("resize", drawChart);
  document.addEventListener("visibilitychange", function(){
    if (!document.hidden) poll();
  });

  poll();
  setInterval(poll, 300);
})();
</script>
</body>
</html>
)PAGE";

#endif // WEBPAGE_H
