#pragma once
#include <Arduino.h>

// Device web UI, served from PROGMEM once the plug is online.
static const char APP_HTML[] PROGMEM = R"=====(<!DOCTYPE html>
<html lang="en"><head>
<meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="color-scheme" content="light dark">
<title>FireLabs S31</title>
<style>
:root{color-scheme:light dark;--red:#FA2800;--orange:#FFA01E;--grad:linear-gradient(135deg,#FA2800,#FF7A12 55%,#FFA01E);
--ink:#1B1613;--muted:#90847b;--card:#fff;--line:#efe4d9;--ls:#e4d6c8;--page:#FBF6F1;--inp:#fffdfb;
--stat:linear-gradient(180deg,#fffdfb,#fbf5ee);--tabon:#1B1613;--tabfg:#fff;--seg:#f4ece4;--segon:#fff;
--good:#28a86a;--goodbg:#e7f6ee;--danger:#d63a1e;--dbd:#f0c9c0;--track:#e3d7ca;--glyph:#f6efe8;--ledoff:#cfc3b6;--co:#fff7ee;--cob:#ffe2bf;--cof:#6b5a45}
@media(prefers-color-scheme:dark){:root{--ink:#f3ebe3;--muted:#9c8e82;--card:#1d1916;--line:#2b2521;--ls:#39312b;--page:#141110;--inp:#171310;
--stat:linear-gradient(180deg,#221d19,#1a1613);--tabon:var(--grad);--seg:#241f1b;--segon:#39312b;--good:#36d484;--goodbg:rgba(54,212,132,.14);
--danger:#ff6a4d;--dbd:rgba(255,106,77,.35);--track:#3a322c;--glyph:#2a241f;--ledoff:#4a4038;--co:rgba(255,160,30,.08);--cob:rgba(255,160,30,.22);--cof:#cdbba6}}
*{box-sizing:border-box}html,body{overflow-x:hidden}body{margin:0;overflow-wrap:anywhere;background:var(--page);color:var(--ink);font-family:"Quicksand",system-ui,Segoe UI,sans-serif;-webkit-font-smoothing:antialiased}
.mono{font-family:"JetBrains Mono",ui-monospace,monospace}
.stage{max-width:560px;margin:24px auto 60px;padding:0 18px}
.top{display:flex;align-items:center;gap:14px;margin-bottom:18px}.top img{width:56px;height:56px;flex:none}.top>div{min-width:0;flex:1}.top .nm{font-weight:700;font-size:19px;line-height:1;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.top .nm small{display:block;font-weight:500;font-size:11.5px;color:var(--muted);margin-top:4px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap}
.pill{display:inline-flex;align-items:center;gap:7px;font-size:12.5px;font-weight:600;padding:5px 11px;border-radius:999px;margin-left:auto;background:var(--goodbg);color:var(--good)}
.pill.off{background:rgba(214,58,30,.12);color:var(--danger)}
.dot{width:8px;height:8px;border-radius:50%;background:currentColor}
.tabs{display:flex;gap:4px;background:var(--card);border:1px solid var(--line);border-radius:13px;padding:5px;margin-bottom:16px}
.tabs button{flex:1;min-width:0;font-family:inherit;font-weight:600;font-size:12px;color:var(--muted);border:0;background:transparent;padding:9px 3px;border-radius:9px;cursor:pointer;white-space:nowrap;overflow:hidden;text-overflow:ellipsis}
@media(max-width:430px){.tabs button{font-size:11px;padding:9px 2px;letter-spacing:-.02em}}
.tabs button.on{background:var(--tabon);color:var(--tabfg)}
.tab{display:none}.tab.on{display:block}
.card{background:var(--card);border:1px solid var(--line);border-radius:16px;box-shadow:0 8px 24px -12px rgba(0,0,0,.18)}.card+.card{margin-top:16px}
.hd{padding:15px 18px 0;font-weight:700;font-size:15px}.csub{padding:2px 18px 0;color:var(--muted);font-size:12.5px}.bd{padding:16px 18px 18px}
label{display:block;margin-top:14px;font-size:12.5px;font-weight:600;color:var(--muted)}label:first-child{margin-top:0}
input,select{width:100%;margin-top:6px;font-family:inherit;font-size:14.5px;color:var(--ink);padding:11px 12px;border:1px solid var(--ls);border-radius:11px;background:var(--inp)}
input:focus,select:focus{outline:none;border-color:var(--orange);box-shadow:0 0 0 3px rgba(255,160,30,.2)}
input[type=range]{padding:0;border:0;background:none;accent-color:var(--orange)}
.help{margin-top:6px;font-size:12px;color:var(--muted)}.host{color:var(--red);font-weight:500;font-family:"JetBrains Mono",monospace}
.btn{font-family:inherit;font-weight:700;font-size:14px;border:0;border-radius:11px;padding:12px 18px;cursor:pointer}
.btn.p{background:var(--grad);color:#fff;box-shadow:0 6px 16px -7px rgba(250,40,0,.7)}.btn.g{background:var(--card);color:var(--ink);border:1px solid var(--ls)}
.btn.d{background:var(--card);color:var(--danger);border:1px solid var(--dbd)}.blk{width:100%}
.relay{display:flex;align-items:center;gap:18px;padding:20px 18px;cursor:pointer}
.glyph{width:58px;height:58px;border-radius:15px;display:grid;place-items:center;flex:none;background:var(--glyph);color:var(--muted);font-size:26px;transition:.3s}
.relay.on .glyph{background:var(--grad);color:#fff;box-shadow:0 8px 22px -8px rgba(250,40,0,.8)}
.relay .meta{flex:1}.relay .st{font-weight:700;font-size:20px}.relay .lbl{color:var(--muted);font-size:12.5px;margin-top:2px}
.sw{width:64px;height:36px;border-radius:999px;background:var(--track);position:relative;transition:.28s;flex:none}
.sw::after{content:"";position:absolute;top:3px;left:3px;width:30px;height:30px;border-radius:50%;background:#fff;box-shadow:0 2px 5px rgba(0,0,0,.2);transition:.28s}
.relay.on .sw{background:var(--grad)}.relay.on .sw::after{left:31px}
.stats{display:grid;grid-template-columns:1fr 1fr;gap:12px;padding:0 18px 18px}
.stat{border:1px solid var(--line);border-radius:13px;padding:13px 14px;background:var(--stat)}
.stat .v{font-family:"JetBrains Mono",monospace;font-weight:700;font-size:23px}.stat .v span{font-size:13px;color:var(--muted);font-weight:500;margin-left:3px}
.stat .k{font-size:11.5px;color:var(--muted);margin-top:3px;text-transform:uppercase;letter-spacing:.07em}
.row{display:flex;align-items:center;gap:12px;padding:13px 0;border-top:1px solid var(--line)}.row:first-child{border-top:0}.row .grow{flex:1;min-width:0}.row .k{font-weight:600;font-size:13.5px}.row .s{color:var(--muted);font-size:12px;margin-top:2px}.row .val{font-family:"JetBrains Mono",monospace;font-size:13px;color:var(--muted);min-width:0;text-align:right}
.seg{display:flex;background:var(--seg);border-radius:10px;padding:4px}.seg button{flex:1;font-family:inherit;font-weight:600;font-size:12.5px;color:var(--muted);border:0;background:transparent;padding:8px;border-radius:7px;cursor:pointer}.seg button.on{background:var(--segon);color:var(--ink)}
.mini{width:46px;height:27px;border-radius:999px;background:var(--track);position:relative;cursor:pointer;transition:.25s;flex:none}.mini::after{content:"";position:absolute;top:3px;left:3px;width:21px;height:21px;border-radius:50%;background:#fff;transition:.25s}.mini.on{background:var(--grad)}.mini.on::after{left:22px}
.callout{background:var(--co);border:1px solid var(--cob);border-radius:12px;padding:13px 15px;font-size:13px;color:var(--cof);margin-bottom:14px}.callout b{color:var(--ink)}
.fault{display:none;background:rgba(214,58,30,.12);border:1px solid var(--dbd);border-radius:12px;padding:13px 15px;font-size:12.5px;color:var(--danger);margin-bottom:16px}.fault.on{display:block}.fault b{display:block;font-size:14px;margin-bottom:3px}
ol{margin:0;padding-left:18px;font-size:13.5px;line-height:1.55;color:var(--muted)}ol b{color:var(--ink)}
.led{width:14px;height:14px;border-radius:50%;flex:none;background:#2b6cff;box-shadow:0 0 9px rgba(43,108,255,.75)}.led.slow{animation:bl 2s steps(1) infinite}.led.fast{animation:bl .34s steps(1) infinite}.led.dim{opacity:.32}.led.o{background:var(--ledoff);box-shadow:none}@keyframes bl{50%{opacity:.12}}
.foot{text-align:center;color:var(--muted);opacity:.7;font-size:11.5px;margin-top:26px}
.toast{position:fixed;left:50%;bottom:24px;transform:translateX(-50%) translateY(80px);background:var(--ink);color:var(--page);padding:11px 18px;border-radius:10px;font-size:13px;font-weight:600;transition:.3s;opacity:0}.toast.show{transform:translateX(-50%);opacity:1}
</style></head><body>
<div class="stage">
<div class="top"><img src="/logo.png" alt="FireLabs"><div><div class="nm" id="nm">FireLabs S31<small class="mono" id="sub">connecting…</small></div></div><span class="pill" id="pill"><span class="dot"></span>…</span></div>

<div class="tabs">
<button class="on" onclick="tb(this,'status')">Status</button><button onclick="tb(this,'mqtt')">MQTT</button>
<button onclick="tb(this,'set')">Settings</button><button id="caltab" onclick="tb(this,'cal')">Calibration</button><button onclick="tb(this,'sys')">System</button></div>

<div class="tab on" id="status">
<div class="fault" id="flt"><b>No load</b>The relay is on but nothing is drawing power. Check the appliance; it may be off, empty, or unplugged.</div>
<div class="card"><div class="relay" id="relay" onclick="toggleRelay()"><div class="glyph">&#9211;</div>
<div class="meta"><div class="st" id="rst">Relay</div><div class="lbl">Tap to toggle · red LED tracks this</div></div><div class="sw"></div></div>
<div class="stats" id="stats"><div class="stat"><div class="v" id="p">--<span>W</span></div><div class="k">Power</div></div>
<div class="stat"><div class="v" id="v">--<span>V</span></div><div class="k">Voltage</div></div>
<div class="stat"><div class="v" id="i">--<span>A</span></div><div class="k">Current</div></div>
<div class="stat"><div class="v" id="e">--<span>kWh</span></div><div class="k">Today</div></div></div></div>
<div class="card" id="live"><div class="hd">Live</div><div class="bd" style="padding-top:10px">
<div style="display:flex;justify-content:space-between;align-items:baseline"><div class="k">Power</div><div class="mono" id="lw" style="font-weight:700">--<span style="color:var(--muted);font-size:12px"> W</span></div></div>
<canvas id="gW" style="width:100%;height:96px;margin-top:6px;display:block"></canvas>
<div style="display:flex;justify-content:space-between;align-items:baseline;margin-top:14px"><div class="k">Voltage</div><div class="mono" id="lv" style="font-weight:700">--<span style="color:var(--muted);font-size:12px"> V</span></div></div>
<canvas id="gV" style="width:100%;height:52px;margin-top:6px;display:block"></canvas>
<div style="display:flex;justify-content:space-between;color:var(--muted);font-size:10px;margin-top:5px"><span>-60s</span><span>-45s</span><span>-30s</span><span>-15s</span><span>now</span></div>
</div></div>
<div class="card"><div class="bd" style="padding-top:16px">
<div class="row"><div class="grow"><div class="k">WiFi signal</div></div><div class="val" id="rssi">--</div></div>
<div class="row"><div class="grow"><div class="k">Uptime</div></div><div class="val" id="up">--</div></div>
<div class="row"><div class="grow"><div class="k">Firmware</div></div><div class="val" id="fw">--</div></div></div></div></div>

<div class="tab" id="mqtt"><div class="card"><div class="hd">Broker</div><div class="csub">Plaintext on your LAN. Home Assistant picks it up by discovery.</div><div class="bd">
<label>Host<input id="mh" class="mono"></label>
<div style="display:flex;gap:12px;margin-top:14px"><label style="flex:1;margin-top:0">Port<input id="mp" type="number" class="mono"></label><label style="flex:2;margin-top:0">Discovery prefix<input id="md" class="mono"></label></div>
<label>Username<input id="mu"></label><label>Password<input id="mpw" type="password" placeholder="(unchanged)"></label>
<button class="btn p blk" style="margin-top:18px" onclick="saveMqtt()">Save</button>
<div class="row" style="margin-top:14px"><span class="pill" id="mst" style="margin:0"><span class="dot"></span>…</span><div class="grow"></div><div class="val" id="mtb"></div></div></div></div></div>

<div class="tab" id="set"><div class="card"><div class="hd">Identity</div><div class="bd">
<label>Device name<input id="dn" oninput="hp()"></label><div class="help">Network name: <span class="host" id="hpv">fl-</span>. The fl- prefix keeps it clear of other devices.</div></div></div>
<div class="card"><div class="hd">Behavior</div><div class="bd">
<label style="margin-bottom:8px">Relay state after power loss</label>
<div class="seg" id="rm"><button onclick="seg(this,0)">Off</button><button onclick="seg(this,1)">On</button><button onclick="seg(this,2)">Last state</button></div>
<div class="row" id="nlrow" style="margin-top:16px"><div class="grow"><div class="k">No-load indicator</div><div class="s">Blue LED glows when the relay is on but nothing draws power</div></div><div class="mini" id="nl" onclick="this.classList.toggle('on')"></div></div>
<label style="margin-top:16px">Indicator brightness<input id="ib" type="range" min="5" max="100" value="30" oninput="bv.textContent=this.value+'%'"></label>
<div class="help"><span id="bv">30%</span></div>
<button class="btn p blk" style="margin-top:18px" onclick="saveSet()">Save settings</button></div></div>
<div class="card" id="ftcard"><div class="hd">Fault detection</div><div class="csub">Flags a load that has stopped drawing power while switched on: a humidifier out of water, an appliance that tripped.</div><div class="bd">
<div class="row" style="padding-top:0"><div class="grow"><div class="k">Enabled</div><div class="s">Reports the fault here and to Home Assistant</div></div><div class="mini" id="fte" onclick="this.classList.toggle('on')"></div></div>
<div style="display:flex;gap:12px;margin-top:14px"><label style="flex:1;margin-top:0">Trip below (W)<input id="ftw" type="number" step="0.1" min="0" max="100" class="mono"></label><label style="flex:1;margin-top:0">Sustained for (s) <span id="ftx" style="font-weight:500;color:var(--red)"></span><input id="fth" type="number" min="10" max="3600" class="mono" oninput="fhv()"></label></div>
<div class="help">Measured on an average, not a single reading, so a load that dips to 0W between cycles won't trip it.</div>
<button class="btn p blk" style="margin-top:18px" onclick="saveSet()">Save settings</button></div></div></div>

<div class="tab" id="cal"><div class="card"><div class="hd">Power calibration</div><div class="csub">The CSE7766 ships uncalibrated. Calibrate once against a known load.</div><div class="bd">
<div class="callout"><b>You'll need</b> a plug-in power meter and a steady load (an incandescent bulb or a heater on a fixed setting).</div>
<ol><li>Plug the meter and load into this outlet, relay <b>ON</b>.</li><li>Let it settle ~30 seconds.</li><li>Enter the meter's true watts and hit Calibrate.</li></ol>
<div class="row" style="margin-top:14px"><div class="grow"><div class="k">Plug reads now</div></div><div class="val" id="calnow" style="font-size:15px">--</div></div>
<label>True power from your meter (W)<input id="cw" type="number" placeholder="e.g. 60.0" class="mono"></label>
<button class="btn p blk" style="margin-top:16px" onclick="calib()">Calibrate</button>
<div class="help" style="text-align:center">Current factor: <span class="mono" id="cf">1.000</span></div></div></div></div>

<div class="tab" id="sys"><div class="card"><div class="hd">Device</div><div class="bd" style="padding-top:14px">
<div class="row"><div class="grow"><div class="k">Firmware</div></div><div class="val" id="fw2">--</div></div>
<div class="row"><div class="grow"><div class="k">Flash</div></div><div class="val" id="fl">--</div></div>
<div class="row"><div class="grow"><div class="k">Free heap</div></div><div class="val" id="heap">--</div></div>
<div class="row"><div class="grow"><div class="k">MAC</div></div><div class="val" id="mac">--</div></div></div></div>
<div class="card"><div class="bd" style="padding-top:16px"><div class="row" style="border:0"><div class="grow"><div class="k">Identify</div><div class="s">Blink the LED to spot this plug; press the plug button to stop</div></div><button class="btn g" id="idbtn" onclick="toggleIdentify()">Identify</button></div></div></div>
<div class="card"><div class="hd">Firmware update</div><div class="bd">
<form method="POST" action="/update" enctype="multipart/form-data"><input type="file" name="firmware" accept=".bin" style="padding:9px"><button class="btn g blk" style="margin-top:12px">Upload &amp; flash</button></form></div></div>
<div class="card"><div class="bd" style="padding-top:16px"><div style="display:flex;gap:10px">
<button class="btn g" style="flex:1" onclick="if(confirm('Restart now?'))post('/api/restart',{})">Restart</button>
<button class="btn d" style="flex:1" onclick="if(confirm('Erase all settings and reboot to setup?'))post('/api/reset',{})">Factory reset</button></div>
<div class="help" style="text-align:center;margin-top:10px">Or hold the plug button for 5 seconds.</div></div></div></div>

<div class="foot">FireLabs S31 · web UI</div></div>
<div class="toast" id="toast"></div>
<script>
function $(i){return document.getElementById(i)}
function tb(b,id){document.querySelectorAll('.tabs button').forEach(x=>x.classList.remove('on'));b.classList.add('on');document.querySelectorAll('.tab').forEach(t=>t.classList.remove('on'));$(id).classList.add('on')}
function toast(m){var t=$('toast');t.textContent=m;t.classList.add('show');setTimeout(()=>t.classList.remove('show'),1800)}
function post(u,o){return fetch(u,{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(o)})}
var identifying=false;
function toggleIdentify(){identifying=!identifying;post('/api/identify',{on:identifying});paintIdentify();}
function paintIdentify(){var b=$('idbtn');if(!b)return;b.textContent=identifying?'Stop identifying':'Identify';b.className='btn '+(identifying?'p':'g');}
function host(v){return 'fl-'+(v.toLowerCase().replace(/[^a-z0-9]+/g,'-').replace(/^-|-$/g,'')||'s31')}
function hp(){$('hpv').textContent=host($('dn').value)}
function fmtUp(s){var d=s/86400|0,h=s%86400/3600|0,m=s%3600/60|0,c=s%60;return (d?d+'d ':'')+String(h).padStart(2,'0')+':'+String(m).padStart(2,'0')+':'+String(c).padStart(2,'0')}
var rmode=2;function seg(b,n){rmode=n;b.parentElement.querySelectorAll('button').forEach(x=>x.classList.remove('on'));b.classList.add('on')}
var relayOn=false;
function toggleRelay(){relayOn=!relayOn;paintRelay();post('/api/relay',{on:relayOn})}
function paintRelay(){$('relay').classList.toggle('on',relayOn);$('rst').textContent='Relay '+(relayOn?'ON':'OFF')}
var histW=[],histV=[];
function chart(id,data,fill){
var c=$(id);if(!c)return;var dpr=window.devicePixelRatio||1,w=c.clientWidth,h=c.clientHeight;if(!w||!h)return;
if(c.width!=w*dpr||c.height!=h*dpr){c.width=w*dpr;c.height=h*dpr;}
var x=c.getContext('2d');x.setTransform(dpr,0,0,dpr,0,0);x.clearRect(0,0,w,h);if(data.length<2)return;
var rmin=Math.min.apply(null,data),rmax=Math.max.apply(null,data),sp=rmax-rmin;if(sp<1e-6)sp=Math.max(1,Math.abs(rmax)*0.1);
var mn=rmin-sp*0.22,mx=rmax+sp*0.22,rg=mx-mn,pad=3,N=30;
function px(i){var fr=(data.length-1)-i;return w-pad-fr*(w-2*pad)/(N-1);}
function py(v){return h-pad-(v-mn)*(h-2*pad)/rg;}
function fmt(v){return Math.abs(v)>=100?v.toFixed(0):v.toFixed(1);}
var cs=getComputedStyle(document.documentElement),mc=(cs.getPropertyValue('--muted')||'#999').trim(),lc=(cs.getPropertyValue('--line')||'#ccc').trim();
x.font='10px ui-monospace,monospace';
[[rmax,'top'],[rmin,'bottom']].forEach(function(p){var yy=py(p[0]);x.strokeStyle=lc;x.lineWidth=1;x.beginPath();x.moveTo(pad,yy);x.lineTo(w-pad,yy);x.stroke();
x.fillStyle=mc;x.textBaseline=p[1];x.fillText(fmt(p[0]),pad+2,p[1]=='top'?yy+2:yy-2);});
if(fill){x.beginPath();x.moveTo(px(0),py(data[0]));for(var i=1;i<data.length;i++)x.lineTo(px(i),py(data[i]));
x.lineTo(px(data.length-1),h);x.lineTo(px(0),h);x.closePath();
var g=x.createLinearGradient(0,0,0,h);g.addColorStop(0,'rgba(250,40,0,.33)');g.addColorStop(1,'rgba(255,160,30,0)');x.fillStyle=g;x.fill();}
x.beginPath();x.moveTo(px(0),py(data[0]));for(var k=1;k<data.length;k++)x.lineTo(px(k),py(data[k]));
x.strokeStyle=fill?'#FA2800':'#FFA01E';x.lineWidth=2;x.lineJoin='round';x.stroke();}
function status(){fetch('/api/status').then(r=>r.json()).then(function(s){
$('nm').firstChild.textContent=s.name||'FireLabs S31';$('sub').textContent=s.host+'.local · '+s.mac;
$('pill').className='pill';$('pill').innerHTML='<span class="dot"></span>Online';
relayOn=s.relay;paintRelay();
$('p').innerHTML=s.power.toFixed(1)+'<span>W</span>';$('v').innerHTML=s.voltage.toFixed(1)+'<span>V</span>';
$('i').innerHTML=s.current.toFixed(2)+'<span>A</span>';$('e').innerHTML=s.energy.toFixed(3)+'<span>kWh</span>';
$('rssi').textContent=s.rssi+' dBm';$('up').textContent=fmtUp(s.uptime);$('fw').textContent='FireLabs '+s.fw;
$('fw2').textContent='FireLabs '+s.fw;$('fl').textContent=(s.flash/1048576|0)+' MB';$('heap').textContent=(s.heap/1024).toFixed(1)+' KB';$('mac').textContent=s.mac;
$('calnow').textContent=s.power.toFixed(1)+' W · '+s.voltage.toFixed(1)+' V · '+s.current.toFixed(2)+' A';
['stats','caltab','nlrow','live','ftcard'].forEach(function(id){var el=$(id);if(el)el.style.display=s.meter?'':'none'});
$('flt').classList.toggle('on',!!s.fault);
if(s.meter){histW.push(s.power);if(histW.length>30)histW.shift();histV.push(s.voltage);if(histV.length>30)histV.shift();
$('lw').innerHTML=s.power.toFixed(1)+'<span style="color:var(--muted);font-size:12px"> W</span>';
$('lv').innerHTML=s.voltage.toFixed(1)+'<span style="color:var(--muted);font-size:12px"> V</span>';
chart('gW',histW,true);chart('gV',histV,false);}
identifying=!!s.identify;paintIdentify();
var ms=$('mst');ms.className='pill'+(s.mqtt?'':' off');ms.innerHTML='<span class="dot"></span>'+(s.mqtt?'Connected':'Disconnected');$('mtb').textContent=s.topic||'';
}).catch(function(){$('pill').className='pill off';$('pill').innerHTML='<span class="dot"></span>Offline'})}
function loadCfg(){fetch('/api/config').then(r=>r.json()).then(function(c){
$('mh').value=c.mqtt_host||'';$('mp').value=c.mqtt_port||1883;$('md').value=c.disc_prefix||'homeassistant';$('mu').value=c.mqtt_user||'';
$('dn').value=c.name||'';hp();rmode=c.restore_mode;$('rm').children[rmode].classList.add('on');
$('nl').classList.toggle('on',c.noload);$('ib').value=c.brightness;$('bv').textContent=c.brightness+'%';$('cf').textContent=(c.cal_p||1).toFixed(3);
$('fte').classList.toggle('on',c.fault_en!==false);$('ftw').value=(c.fault_w!==undefined?c.fault_w:0.5);$('fth').value=c.fault_hold||300;fhv()})}
function fhv(){var s=+$('fth').value||0;$('ftx').textContent=s>=60?'= '+(s/60).toFixed(s%60?1:0)+' min':''}
function saveMqtt(){var o={mqtt_host:$('mh').value,mqtt_port:+$('mp').value,disc_prefix:$('md').value,mqtt_user:$('mu').value};if($('mpw').value)o.mqtt_pass=$('mpw').value;post('/api/config',o).then(()=>toast('MQTT saved — reconnecting'))}
function saveSet(){post('/api/config',{name:$('dn').value,restore_mode:rmode,noload:$('nl').classList.contains('on'),brightness:+$('ib').value,fault_en:$('fte').classList.contains('on'),fault_w:+$('ftw').value,fault_hold:+$('fth').value}).then(()=>toast('Settings saved'))}
function calib(){var w=parseFloat($('cw').value);if(!w){toast('Enter the meter watts');return}post('/api/calibrate',{watts:w}).then(r=>r.json()).then(function(r){$('cf').textContent=(r.cal_p||1).toFixed(3);toast('Calibrated')})}
status();loadCfg();setInterval(status,2000);
</script></body></html>)=====";
