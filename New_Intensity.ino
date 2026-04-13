#include <WiFi.h>
#include <ESPAsyncWebServer.h>


const char* ssid = "wifi name";
const char* password = "Password";

// Pins - LDR and LEDs
#define LDR_SENSOR 34   // Reading room luminosity
int ledPins[] = {13, 12, 14, 27}; // All home LEDs

// PWM Settings
const int freq = 5000;
const int resolution = 8; // 0-255 levels

// Global Variables
int currentBrightness = 128; 
bool manualMode = false;
unsigned long lastUpdate = 0;

AsyncWebServer server(80);

// Glassmorphism UI with Slider
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8"><meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.15.4/css/all.css">
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;600&display=swap" rel="stylesheet">
  <title>Sentinel | Light Control</title>
  <style>
    :root { --bg: #0d1117; --primary: #58a6ff; --on: #39d353; --accent: #f2cc60; }
    body { font-family: 'Poppins', sans-serif; background: #0d1117; color: #c9d1d9; display: flex; flex-direction: column; align-items: center; padding: 40px; }
    .glass-card { 
      background: rgba(22, 27, 34, 0.8); backdrop-filter: blur(10px); 
      border: 1px solid rgba(255,255,255,0.1); padding: 40px; border-radius: 25px; 
      text-align: center; width: 100%; max-width: 500px; box-shadow: 0 8px 32px rgba(0,0,0,0.5);
    }
    h1 { background: linear-gradient(to right, #58a6ff, #bc85ff); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .btn { padding: 12px 25px; border-radius: 12px; border: none; cursor: pointer; font-weight: 600; margin-bottom: 20px; transition: 0.3s; }
    .auto { background: var(--on); color: white; }
    .man { background: var(--accent); color: black; }
    input[type=range] { width: 100%; height: 12px; border-radius: 10px; background: #30363d; outline: none; -webkit-appearance: none; margin: 20px 0; }
    input[type=range]::-webkit-slider-thumb { -webkit-appearance: none; width: 25px; height: 25px; border-radius: 50%; background: var(--primary); cursor: pointer; box-shadow: 0 0 15px var(--primary); }
    .status { font-size: 0.8rem; color: #8b949e; letter-spacing: 1px; }
  </style>
</head>
<body>
  <div class="glass-card">
    <h1>LUX CONTROL</h1>
    <p class="status">SMART HOME LIGHTING SYSTEM</p>
    
    <button id="modeBtn" class="btn auto" onclick="toggleMode()">MODE: AUTO-BRIGHTNESS</button>
    
    <input type="range" min="0" max="255" value="128" id="dimmer" oninput="updateDimmer(this.value)">
    
    <div style="font-size: 1.5rem;">Intensity: <span id="brightVal" style="color:var(--primary)">128</span></div>
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
    // Sync brightness value even in Auto Mode
    setInterval(function(){
      fetch('/val').then(r => r.text()).then(v => {
        document.getElementById("brightVal").innerHTML = v;
        document.getElementById("dimmer").value = v;
      });
    }, 1000);
  </script>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);

  // Attach all LEDs to PWM channels
  for (int i = 0; i < 4; i++) {
    ledcAttach(ledPins[i], freq, resolution);
  }

  pinMode(LDR_SENSOR, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  // Web Routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/slider", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("value")) {
      currentBrightness = request->getParam("value")->value().toInt();
      manualMode = true; // Switching to manual when user touches slider
    }
    request->send(200, "text/plain", "OK");
  });

  server.on("/toggleMode", HTTP_GET, [](AsyncWebServerRequest *request){
    manualMode = !manualMode;
    request->send(200, "text/plain", manualMode ? "Manual" : "Auto");
  });

  server.on("/val", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(currentBrightness));
  });

  server.begin();
}

void loop() {
  if (!manualMode) {
    // AUTO MODE: Read LDR and calculate brightness
    // Dark room (LDR low) -> Bright LED (PWM high)
    int ldrReading = analogRead(LDR_SENSOR);
    currentBrightness = map(ldrReading, 0, 4095, 255, 0); 
  }

  // Apply brightness to all LEDs
  for (int i = 0; i < 4; i++) {
    ledcWrite(ledPins[i], currentBrightness);
  }

  delay(50); // Small delay for stability
}
