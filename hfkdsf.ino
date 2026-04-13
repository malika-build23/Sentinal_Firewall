#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// ========================
// CONFIGURATION
// ========================
const char* ssid = "Infinix NOTE 40 Pro 5G";
const char* password = "786Malikaa";

// Pins
#define MQ4_PIN 39    
#define sensor1 25    
#define sensor2 26    
#define LDR1 34       
#define LDR2 35       

int ledPins[] = {13, 12, 14, 27}; 

// PWM Settings
const int freq = 5000;
const int resolution = 8; // 0-255

// Variables
int gasValue = 0, count = 0, currentBrightness = 0; 
bool manualMode = false;
String serialMirror = "System Online";
unsigned long lastUpdate = 0;

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.15.4/css/all.css">
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
  <title>Sentinel | Full Intensity Control</title>
  <style>
    :root { --bg: #0d1117; --card: #161b22; --primary: #58a6ff; --on: #39d353; --accent: #f2cc60; }
    body { font-family: 'Poppins', sans-serif; background: var(--bg); color: #c9d1d9; display: flex; flex-direction: column; align-items: center; padding: 20px; }
    .glass-card { background: rgba(22, 27, 34, 0.8); backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.1); padding: 30px; border-radius: 25px; text-align: center; width: 100%; max-width: 600px; box-shadow: 0 8px 32px rgba(0,0,0,0.5); }
    h1 { background: linear-gradient(to right, #58a6ff, #bc85ff); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 15px; margin: 20px 0; }
    .stat-box { background: rgba(255,255,255,0.05); padding: 15px; border-radius: 15px; }
    .val { font-size: 1.8rem; font-weight: 600; color: #fff; }
    .btn { padding: 12px 25px; border-radius: 12px; border: none; cursor: pointer; font-weight: 600; margin-bottom: 20px; transition: 0.3s; width: 100%; }
    .auto { background: var(--on); color: white; } .man { background: var(--accent); color: black; }
    input[type=range] { width: 100%; height: 12px; border-radius: 10px; background: #30363d; outline: none; -webkit-appearance: none; margin: 20px 0; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 25px; height: 25px; border-radius: 50%; background: var(--primary); cursor: pointer; box-shadow: 0 0 15px var(--primary); }
    .log { width: 100%; background: #000; padding: 15px; border-radius: 10px; margin-top: 20px; color: var(--on); font-family: monospace; font-size: 0.8rem; text-align: left; border-left: 3px solid var(--primary); min-height: 50px;}
  </style>
</head>
<body>
  <div class="glass-card">
    <h1>SENTINEL FIREWALL</h1>
    <div class="grid">
      <div class="stat-box"><div style="font-size:0.7rem;">OCCUPANCY</div><div class="val" id="count">0</div></div>
      <div class="stat-box"><div style="font-size:0.7rem;">GAS LEVEL</div><div class="val" id="gas">0</div></div>
    </div>
    <button id="modeBtn" class="btn auto" onclick="toggleMode()">MODE: AUTO-INTENSITY</button>
    <input type="range" min="0" max="255" value="0" id="dimmer" oninput="updateDimmer(this.value)">
    <div style="font-size: 1.2rem;">Intensity: <span id="brightVal" style="color:var(--primary)">0</span></div>
    <div class="log"><strong>SYSTEM LOG:</strong> <span id="mirror">Syncing...</span></div>
  </div>
  <script>
    function updateDimmer(val) {
      document.getElementById("brightVal").innerHTML = val;
      fetch(`/slider?value=${val}`);
    }
    function toggleMode() {
      fetch('/toggleMode').then(r => r.text()).then(m => {
        const btn = document.getElementById("modeBtn");
        btn.innerHTML = "MODE: " + m.toUpperCase();
        btn.className = "btn " + (m == "Manual" ? "man" : "auto");
      });
    }
    setInterval(function(){
      fetch('/data').then(r => r.json()).then(d => {
        document.getElementById("count").innerHTML = d.count;
        document.getElementById("gas").innerHTML = d.gas;
        document.getElementById("mirror").innerHTML = d.log;
        if(d.mode == "Auto") {
          document.getElementById("brightVal").innerHTML = d.bright;
          document.getElementById("dimmer").value = d.bright;
        }
      });
    }, 500);
  </script>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 4; i++) { ledcAttach(ledPins[i], freq, resolution); }
  pinMode(MQ4_PIN, INPUT); pinMode(sensor1, INPUT_PULLUP); pinMode(sensor2, INPUT_PULLUP);
  pinMode(LDR1, INPUT); pinMode(LDR2, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", index_html); });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"gas\":\"" + String(gasValue) + "\",\"count\":\"" + String(count) + 
                  "\",\"log\":\"" + serialMirror + "\",\"bright\":\"" + String(currentBrightness) + 
                  "\",\"mode\":\"" + (manualMode ? "Manual" : "Auto") + "\"}";
    request->send(200, "application/json", json);
  });
  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      currentBrightness = request->getParam("value")->value().toInt();
      manualMode = true;
    }
    request->send(200, "text/plain", "OK");
  });
  server.on("/toggleMode", HTTP_GET, [](AsyncWebServerRequest *request){
    manualMode = !manualMode;
    request->send(200, "text/plain", manualMode ? "Manual" : "Auto");
  });
  server.begin();
}

void loop() {
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();
    gasValue = analogRead(MQ4_PIN);
  }

  // --- 1. GAS PRIORITY ---
  if (gasValue > 3500) {
    currentBrightness = 255;
    serialMirror = "⚠️ GAS HAZARD! Lights 100%";
  } 
  else {
    // --- 2. OCCUPANCY CHECK ---
    if (count == 0) {
      currentBrightness = 0; // Force OFF
      serialMirror = "Empty House: Energy Saver Active";
    } 
    else {
      // --- 3. AUTO INTENSITY LOGIC ---
      if (!manualMode) {
        int l1 = analogRead(LDR1);
        int l2 = analogRead(LDR2);
        int ldrAvg = (l1 + l2) / 2;

        if (ldrAvg > 2500) { // Using your threshold
          // Room is dark, adjust brightness proportionally
          currentBrightness = map(ldrAvg, 2500, 4095, 50, 255); 
          serialMirror = "Auto-Luminosity: Adjusting for Darkness";
        } else {
          // Room is bright enough
          currentBrightness = 0;
          serialMirror = "Sufficient Daylight: LEDs OFF";
        }
      }
    }
  }

  // Apply to all LEDs
  for (int i = 0; i < 4; i++) {
    ledcWrite(ledPins[i], currentBrightness);
  }

  // --- 4. IR SENSORS ---
  if (digitalRead(sensor1) == LOW) {
    unsigned long start = millis();
    while (digitalRead(sensor2) == HIGH && (millis() - start < 1500)); 
    if(digitalRead(sensor2) == LOW) { count++; delay(500); }
  }
  if (digitalRead(sensor2) == LOW) {
    unsigned long start = millis();
    while (digitalRead(sensor1) == HIGH && (millis() - start < 1500)); 
    if(digitalRead(sensor1) == LOW) { if (count > 0) count--; delay(500); }
  }
}
