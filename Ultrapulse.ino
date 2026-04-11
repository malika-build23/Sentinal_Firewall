#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>


const char* ssid = "Nmae of your wifi";
const char* password = "Wifi-Password";

// Pins - Environment & IR
#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ4_PIN 39
#define sensor1 25
#define sensor2 26

// Pins - Room Lighting (LDRs & LEDs)
#define LDR1 34
#define LDR2 35
int room1[] = {13, 12}; 
int room2[] = {14, 27}; 

// Pins - RFID & Door Servo
#define SS_PIN 5
#define RST_PIN 2
MFRC522 rfid(SS_PIN, RST_PIN);
Servo doorServo;
int doorServoPin = 15;

// Pins - Solar Tracker
#define LDR_LEFT 36   
#define LDR_RIGHT 33
Servo solarServo;
int solarServoPin = 22;

// Global Variables
float temp = 0, hum = 0;
int gasValue = 0, solarPos = 90;
int count = 0; // Back to single count logic
String serialMirror = "System Online"; 
String r1_Status = "OFF", r2_Status = "OFF", doorStatus = "Closed", solarDir = "Stationary";
unsigned long lastUpdate = 0;

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

// Professional Dashboard HTML
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;700&display=swap" rel="stylesheet">
  <title>Sentinel Firewall</title>
  <style>
    :root { --bg: #0b0e14; --card: #161b22; --primary: #58a6ff; --text: #c9d1d9; --on: #238636; --off: #da3633; --accent: #f6c23e; }
    body { font-family: 'Inter', sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 20px; display: flex; flex-direction: column; align-items: center; }
    .header-line { color: #8b949e; font-size: 1rem; margin-bottom: 5px; }
    h1 { color: var(--primary); margin: 5px 0; text-transform: uppercase; letter-spacing: 2px; font-size: 1.8rem; text-align: center;}
    h3 { color: #8b949e; margin-bottom: 25px; font-weight: 400; font-size: 1rem; }
    .container { display: grid; grid-template-columns: repeat(auto-fit, minmax(180px, 1fr)); gap: 15px; width: 100%; max-width: 1000px; }
    .card { background: var(--card); padding: 15px; border-radius: 12px; border: 1px solid #30363d; text-align: center; }
    .card i { font-size: 1.5rem; margin-bottom: 8px; color: var(--primary); }
    .label { font-size: 0.65rem; color: #8b949e; text-transform: uppercase; font-weight: bold; }
    .value { font-size: 1.8rem; font-weight: 700; margin: 5px 0; }
    .status { font-weight: bold; padding: 4px 8px; border-radius: 4px; font-size: 0.85rem; margin-top: 5px; display: inline-block; }
    .ON, .Open { background: var(--on); color: white; }
    .OFF, .Closed, .Denied { background: var(--off); color: white; }
    .serial-box { width: 100%; max-width: 1000px; background: #000; padding: 15px; border-radius: 8px; margin-top: 20px; border: 1px solid #30363d; color: #39ff14; font-family: 'Courier New', monospace; font-size: 1rem; text-align: center; }
    .features { width: 100%; max-width: 1000px; background: var(--card); border: 1px solid #30363d; border-radius: 12px; margin-top: 20px; padding: 20px; box-sizing: border-box; }
    .features h4 { color: var(--primary); margin: 0 0 15px 0; text-transform: uppercase; font-size: 0.9rem; border-bottom: 1px solid #30363d; padding-bottom: 5px;}
    .feature-list { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 10px; font-size: 0.85rem; color: #c9d1d9; }
    footer { margin-top: 40px; text-align: center; color: #8b949e; font-size: 0.8rem; padding-bottom: 20px; }
    .heart { color: #f85149; }
  </style>
</head>
<body>
  <div class="header-line">House Secured by Design: From Code to Comfort !!</div>
  <h1>SENTINEL FIREWALL</h1>
  <h3>Team UltraPulse</h3>

  <div class="container">
    <div class="card"><i class="fas fa-thermometer-half" style="color:#ff6b6b"></i><div class="label">Temperature</div><div class="value"><span id="temp">0</span>°C</div></div>
    <div class="card"><i class="fas fa-tint" style="color:#58a6ff"></i><div class="label">Humidity</div><div class="value"><span id="hum">0</span>%</div></div>
    <div class="card"><i class="fas fa-biohazard" style="color:var(--accent)"></i><div class="label">Gas (MQ4)</div><div class="value"><span id="gas">0</span></div></div>
    <div class="card"><i class="fas fa-users" style="color:var(--accent)"></i><div class="label">Live Count</div><div class="value"><span id="count">0</span></div></div>
    
    <div class="card"><i class="fas fa-key"></i><div class="label">Door Access</div><br><div id="door_stat" class="status Closed">Closed</div></div>
    <div class="card"><i class="fas fa-lightbulb"></i><div class="label">Room 1</div><br><div id="r1_stat" class="status OFF">OFF</div></div>
    <div class="card"><i class="fas fa-lightbulb"></i><div class="label">Room 2</div><br><div id="r2_stat" class="status OFF">OFF</div></div>
    <div class="card"><i class="fas fa-sun" style="color:var(--accent)"></i><div class="label">Solar Panel</div><div class="value"><span id="solar_pos">90</span>°</div><div id="solar_dir" style="color:#58a6ff; font-size:0.75rem;">Stationary</div></div>
  </div>

  <div class="serial-box"><strong>SYSTEM LOG:</strong> <span id="mirror">Initializing...</span></div>

  <div class="features">
    <h4>System Features</h4>
    <div class="feature-list">
      <div>[+] RFID Door Access Control</div>
      <div>[+] Automatic Solar Tracking</div>
      <div>[+] Sequential Entry/Exit Counter</div>
      <div>[+] LDR Smart Room Lighting</div>
      <div>[+] Environmental Gas & Temp Monitor</div>
      <div>[+] Live Web Dashboard (Dark Mode)</div>
    </div>
  </div>

  <footer>
    Made with <span class="heart">&hearts;</span> by Team <strong>UltraPulse</strong><br>
    <strong>Members:</strong> Anurag Verma &middot; Anvi Sharma &middot; Malika Parveen &middot; Trisha Pandey
  </footer>

  <script>
    setInterval(function ( ) {
      var xhttp = new XMLHttpRequest();
      xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
          var data = JSON.parse(this.responseText);
          document.getElementById("temp").innerHTML = data.temp;
          document.getElementById("hum").innerHTML = data.hum;
          document.getElementById("gas").innerHTML = data.gas;
          document.getElementById("count").innerHTML = data.count;
          document.getElementById("mirror").innerHTML = data.mirror;
          document.getElementById("solar_pos").innerHTML = data.spos;
          document.getElementById("solar_dir").innerHTML = data.sdir;
          document.getElementById("r1_stat").innerHTML = data.r1;
          document.getElementById("r1_stat").className = "status " + data.r1;
          document.getElementById("r2_stat").innerHTML = data.r2;
          document.getElementById("r2_stat").className = "status " + data.r2;
          document.getElementById("door_stat").innerHTML = data.door;
          document.getElementById("door_stat").className = "status " + data.door;
        }
      };
      xhttp.open("GET", "/data", true);
      xhttp.send();
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
  solarServo.attach(solarServoPin, 500, 2400); solarServo.write(solarPos);

  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"temp\":\"" + String(temp, 1) + "\",\"hum\":\"" + String(hum, 1) + "\",\"gas\":\"" + String(gasValue) + 
                  "\",\"count\":\"" + String(count) + "\",\"mirror\":\"" + serialMirror + 
                  "\",\"r1\":\"" + r1_Status + "\",\"r2\":\"" + r2_Status + 
                  "\",\"door\":\"" + doorStatus + "\",\"spos\":\"" + String(solarPos) + "\",\"sdir\":\"" + solarDir + "\"}";
    request->send(200, "application/json", json);
  });
  server.begin();
}

void loop() {
  if (millis() - lastUpdate >= 5000) {
    lastUpdate = millis();
    gasValue = analogRead(MQ4_PIN); temp = dht.readTemperature(); hum = dht.readHumidity();
  }

  // Room Light Logic
  int l1 = analogRead(LDR1); int l2 = analogRead(LDR2);
  if(l1 > 2500) { digitalWrite(room1[0], HIGH); digitalWrite(room1[1], HIGH); r1_Status = "ON"; } else { digitalWrite(room1[0], LOW); digitalWrite(room1[1], LOW); r1_Status = "OFF"; }
  if(l2 > 2500) { digitalWrite(room2[0], HIGH); digitalWrite(room2[1], HIGH); r2_Status = "ON"; } else { digitalWrite(room2[0], LOW); digitalWrite(room2[1], LOW); r2_Status = "OFF"; }

  // Solar Tracker
  int leftL = analogRead(LDR_LEFT); int rightR = analogRead(LDR_RIGHT);
  int diff = leftL - rightR;
  if (abs(diff) > 80) {
    if (leftL > rightR) { solarPos--; solarDir = "Moving Left"; } else { solarPos++; solarDir = "Moving Right"; }
    solarPos = constrain(solarPos, 0, 180); solarServo.write(solarPos);
  } else { solarDir = "Stationary"; }

  // RFID Access
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    String id = "";
    for (byte i = 0; i < rfid.uid.size; i++) { id += String(rfid.uid.uidByte[i] < 0x10 ? "0" : ""); id += String(rfid.uid.uidByte[i], HEX); }
    id.toUpperCase();
    if (id == "C4AB6A05") {
      serialMirror = "ACCESS GRANTED - Door Opening"; doorStatus = "Open"; doorServo.write(90);
      delay(3000); doorServo.write(0); doorStatus = "Closed"; serialMirror = "Door Closed";
    } else { serialMirror = "ACCESS DENIED"; doorStatus = "Denied"; }
    rfid.PICC_HaltA(); rfid.PCD_StopCrypto1();
  }

  // ENTRY/EXIT Logic - Back to original count logic
  if (digitalRead(sensor1) == LOW) {
    unsigned long t = millis();
    while (digitalRead(sensor2) == HIGH && millis() - t < 1500);
    if (digitalRead(sensor2) == LOW) { 
      count++; 
      serialMirror = "Entry -> Count: " + String(count); // Exact serial style
      Serial.println(serialMirror); 
      delay(500); 
    }
  }
  if (digitalRead(sensor2) == LOW) {
    unsigned long t = millis();
    while (digitalRead(sensor1) == HIGH && millis() - t < 1500);
    if (digitalRead(sensor1) == LOW) { 
      if (count > 0) count--; 
      serialMirror = "Exit -> Count: " + String(count); // Exact serial style
      Serial.println(serialMirror); 
      delay(500); 
    }
  }
}
