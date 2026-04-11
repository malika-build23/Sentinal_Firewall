#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ========================
// CONFIGURATION
// ========================
const char* ssid = "Wifi Namw";
const char* password = "Password";

#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ4_PIN 39   
#define sensor1 25
#define sensor2 26
#define LDR1 34
#define LDR2 35
int room1[] = {13, 12}; 
int room2[] = {14, 27}; 

#define SS_PIN 5
#define RST_PIN 2
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;
int doorServoPin = 15;

#define LDR_LEFT 32
#define LDR_RIGHT 33
Servo solarServo;
int solarServoPin = 22;
int servoPos = 90; 

float temp = 0, hum = 0;
int gasValue = 0, count = 0; 
String serialMirror = "System Online"; 
String r1_Status = "OFF", r2_Status = "OFF", doorStatus = "Closed", solarDir = "Stationary";
unsigned long lastUpdate = 0;

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.15.4/css/all.css">
  <link href="https://fonts.googleapis.com/css2?family=Poppins:wght@300;400;600&display=swap" rel="stylesheet">
  <title>Sentinel Firewall | UltraPulse</title>
  <style>
    :root { --bg: #0d1117; --card-bg: rgba(22, 27, 34, 0.8); --primary: #58a6ff; --accent: #f2cc60; --on: #39d353; --off: #f85149; --text: #c9d1d9; }
    body { font-family: 'Poppins', sans-serif; background: radial-gradient(circle at top right, #161b22, #0d1117); color: var(--text); margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; }
    .header-container { text-align: center; margin-bottom: 30px; }
    .header-line { color: var(--primary); font-size: 0.9rem; letter-spacing: 2px; text-transform: uppercase; font-weight: 600; opacity: 0.8; }
    h1 { font-size: 2.5rem; margin: 10px 0; background: linear-gradient(to right, #58a6ff, #bc85ff); -webkit-background-clip: text; -webkit-text-fill-color: transparent; }
    .container { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 20px; width: 100%; max-width: 1100px; }
    .card { background: var(--card-bg); backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.1); padding: 25px; border-radius: 20px; text-align: center; transition: transform 0.3s ease; box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.3); }
    .card:hover { transform: translateY(-5px); }
    .label { font-size: 0.75rem; color: #8b949e; text-transform: uppercase; letter-spacing: 1px; margin-bottom: 10px; }
    .value { font-size: 2.2rem; font-weight: 600; color: #fff; }
    .status-pill { font-weight: 600; padding: 6px 15px; border-radius: 50px; font-size: 0.8rem; margin-top: 15px; display: inline-block; }
    .ON, .Open { background: rgba(57, 211, 83, 0.15); color: var(--on); border: 1px solid var(--on); }
    .OFF, .Closed, .Denied { background: rgba(248, 81, 73, 0.15); color: var(--off); border: 1px solid var(--off); }
    .serial-box { width: 100%; max-width: 1100px; background: #000; padding: 20px; border-radius: 15px; margin-top: 30px; border: 1px solid #30363d; color: var(--on); font-family: 'Courier New', monospace; border-left: 4px solid var(--primary); }
    .features-section { width: 100%; max-width: 1100px; background: var(--card-bg); border-radius: 20px; margin-top: 30px; padding: 30px; }
    .feature-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 15px; }
    .feature-item i { color: var(--primary); margin-right: 15px; }
    footer { margin-top: 50px; padding: 30px; text-align: center; border-top: 1px solid rgba(255,255,255,0.05); width: 100%; }
  </style>
</head>
<body>
  <div class="header-container">
    <div class="header-line">House Secured by Design: From Code to Comfort</div>
    <h1>SENTINEL FIREWALL</h1>
    <h3 style="font-weight:300; color:#8b949e">Engineering by <span style="color:var(--primary)">Team UltraPulse</span></h3>
  </div>

  <div class="container">
    <div class="card"><i class="fas fa-temperature-high" style="color:#ff6b6b"></i><div class="label">Atmosphere</div><div class="value"><span id="temp">0</span>&deg;C</div></div>
    <div class="card"><i class="fas fa-water" style="color:#58a6ff"></i><div class="label">Humidity</div><div class="value"><span id="hum">0</span>%</div></div>
    <div class="card"><i class="fas fa-wind" style="color:var(--accent)"></i><div class="label">Air Purity</div><div class="value"><span id="gas">0</span></div></div>
    <div class="card"><i class="fas fa-walking" style="color:#bc85ff"></i><div class="label">Occupancy</div><div class="value"><span id="count">0</span></div></div>
    <div class="card"><i class="fas fa-fingerprint"></i><div class="label">Entrance</div><div id="door_stat" class="status-pill Closed">Secure</div></div>
    <div class="card"><i class="fas fa-solar-panel" style="color:var(--accent)"></i><div class="label">Solar Tracking</div><div class="value"><span id="solar_pos">90</span>&deg;</div><div id="solar_dir" style="color:var(--primary); font-size:0.8rem; margin-top:5px">Stationary</div></div>
    <div class="card"><i class="fas fa-lightbulb"></i><div class="label">Room Alpha</div><div id="r1_stat" class="status-pill OFF">OFF</div></div>
    <div class="card"><i class="fas fa-lightbulb"></i><div class="label">Room Beta</div><div id="r2_stat" class="status-pill OFF">OFF</div></div>
  </div>

  <div class="serial-box"><i class="fas fa-terminal" style="margin-right:10px"></i><strong>LIVE LOG:</strong> <span id="mirror" style="margin-left:15px">Awaiting hardware interrupt...</span></div>

  <div class="features-section">
    <h4 style="margin-top:0; color:var(--primary)">SYSTEM ARCHITECTURE</h4>
    <div class="feature-grid">
      <div class="feature-item"><i class="fas fa-shield-alt"></i> RFID Encrypted Door Access</div>
      <div class="feature-item"><i class="fas fa-sun"></i> Dual-Axis Solar Optimizer</div>
      <div class="feature-item"><i class="fas fa-user-check"></i> Smart Bi-Directional Counter</div>
      <div class="feature-item"><i class="fas fa-bolt"></i> Auto-Light Energy Saver</div>
      <div class="feature-item"><i class="fas fa-exclamation-triangle"></i> Hazard Gas Detection</div>
      <div class="feature-item"><i class="fas fa-cloud"></i> IoT Glass UI Dashboard</div>
    </div>
  </div>

  <footer>Made by Team <strong>UltraPulse</strong><br><span style="font-size:0.7rem; opacity:0.6">Anurag Verma &bull; Anvi Sharma &bull; Malika Parveen &bull; Trisha Pandey</span></footer>

  <script>
    setInterval(function(){
      fetch('/data').then(r => r.json()).then(data => {
        document.getElementById("temp").innerHTML = data.temp;
        document.getElementById("hum").innerHTML = data.hum;
        document.getElementById("gas").innerHTML = data.gas;
        document.getElementById("count").innerHTML = data.count;
        document.getElementById("mirror").innerHTML = data.mirror;
        document.getElementById("solar_pos").innerHTML = data.spos;
        document.getElementById("solar_dir").innerHTML = data.sdir;
        const updateStatus = (id, val, prefix) => { const el = document.getElementById(id); el.innerHTML = prefix + " " + val; el.className = "status-pill " + val; };
        updateStatus("r1_stat", data.r1, "A:"); updateStatus("r2_stat", data.r2, "B:"); updateStatus("door_stat", data.door, "DOOR:");
      });
    }, 400);
  </script>
</body></html>)rawliteral";

void setup() {
  Serial.begin(115200);
  pinMode(sensor1, INPUT_PULLUP); pinMode(sensor2, INPUT_PULLUP);
  pinMode(MQ4_PIN, INPUT); pinMode(LDR1, INPUT); pinMode(LDR2, INPUT);
  for (int i = 0; i < 2; i++) { pinMode(room1[i], OUTPUT); pinMode(room2[i], OUTPUT); }
  SPI.begin(18, 19, 23, 5);
  rfid.PCD_Init();
  doorServo.attach(doorServoPin, 500, 2400); doorServo.write(0);
  solarServo.attach(solarServoPin, 500, 2400); solarServo.write(servoPos);
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ request->send_P(200, "text/html", index_html); });
  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temp\":\"" + String(temp, 1) + "\",\"hum\":\"" + String(hum, 1) + "\",\"gas\":\"" + String(gasValue) + 
                  "\",\"count\":\"" + String(count) + "\",\"mirror\":\"" + serialMirror + 
                  "\",\"r1\":\"" + r1_Status + "\",\"r2\":\"" + r2_Status + 
                  "\",\"door\":\"" + doorStatus + "\",\"spos\":\"" + String(servoPos) + "\",\"sdir\":\"" + solarDir + "\"}";
    request->send(200, "application/json", json);
  });
  server.begin();
}

void loop() {
  if (millis() - lastUpdate >= 5000) {
    lastUpdate = millis();
    gasValue = analogRead(MQ4_PIN); temp = dht.readTemperature(); hum = dht.readHumidity();
  }

  // --- ROOM LIGHTS ---
  int l1 = analogRead(LDR1); int l2 = analogRead(LDR2);
  if(l1 > 2500) { digitalWrite(room1[0], HIGH); digitalWrite(room1[1], HIGH); r1_Status = "ON"; } else { digitalWrite(room1[0], LOW); digitalWrite(room1[1], LOW); r1_Status = "OFF"; }
  if(l2 > 2500) { digitalWrite(room2[0], HIGH); digitalWrite(room2[1], HIGH); r2_Status = "ON"; } else { digitalWrite(room2[0], LOW); digitalWrite(room2[1], LOW); r2_Status = "OFF"; }

  // --- SOLAR TRACKER ---
  int leftVal = analogRead(LDR_LEFT); int rightVal = analogRead(LDR_RIGHT);
  int diff = leftVal - rightVal;
  if (abs(diff) > 80) {
    if (leftVal > rightVal) { servoPos--; solarDir = "Moving Left"; } else { servoPos++; solarDir = "Moving Right"; }
    servoPos = constrain(servoPos, 0, 180); solarServo.write(servoPos);
    delay(20);
  } else { solarDir = "Stationary"; }

  // --- RFID DOOR ---
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String id = "";
    for (byte i = 0; i < rfid.uid.size; i++) { id += String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); id += String(rfid.uid.uidByte[i], HEX); }
    id.toUpperCase();
    if (id == "C4AB6A05") {
      serialMirror = "ACCESS GRANTED"; doorStatus = "Open"; doorServo.write(90); delay(3000); 
      doorServo.write(0); doorStatus = "Closed"; serialMirror = "Door Locked";
    } else { serialMirror = "ACCESS DENIED"; doorStatus = "Denied"; }
    rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
  }

  // --- YOUR UPDATED IR SENSOR LOGIC ---
  // ENTRY
  if (digitalRead(sensor1) == LOW) {
    while (digitalRead(sensor2) == HIGH); // Wait for sensor 2 to be triggered
    count++;
    serialMirror = "Entry Detected → Count: " + String(count);
    Serial.println(serialMirror);
    delay(500);
  }

  // EXIT
  if (digitalRead(sensor2) == LOW) {
    while (digitalRead(sensor1) == HIGH); // Wait for sensor 1 to be triggered
    if (count > 0) { count--; }
    serialMirror = "Exit Detected → Count: " + String(count);
    Serial.println(serialMirror);
    delay(500);
  }
}
