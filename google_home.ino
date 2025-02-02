#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

ESP8266WebServer server(80);

// WiFi credentials
const char* ssid = "Redmi 9 Prime"; 
const char* pass = "25418125"; 

// Define component pins
#define led1 D1
#define led2 D2
#define trig D4
#define echo D5
#define DHTPIN D3
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

float temperature = 0.0, humidity = 0.0, distance = 0.0;

void setup() {
  Serial.begin(9600);

  // Initialize pins
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  // Initialize DHT sensor
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(3000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/led1/on", []() {
    digitalWrite(led1, HIGH);
    server.send(200, "text/plain", "LED 1 is ON");
  });
  server.on("/led1/off", []() {
    digitalWrite(led1, LOW);
    server.send(200, "text/plain", "LED 1 is OFF");
  });
  server.on("/led2/on", []() {
    digitalWrite(led2, HIGH);
    server.send(200, "text/plain", "LED 2 is ON");
  });
  server.on("/led2/off", []() {
    digitalWrite(led2, LOW);
    server.send(200, "text/plain", "LED 2 is OFF");
  });
  server.on("/sensorData", sendSensorData);

  server.begin();
}

void loop() {
  server.handleClient();
  readSensors();
}

void readSensors() {
  // Read temperature and humidity
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    humidity = h;
    temperature = t;
  } else {
    Serial.println("Failed to read from DHT sensor!");
  }

  // Read distance using ultrasonic sensor
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long duration = pulseIn(echo, HIGH, 30000); // Timeout after 30ms
  if (duration > 0) {
    distance = duration / 29.0 / 2.0;
  } else {
    Serial.println("Failed to read distance!");
    distance = -1.0; // Indicate error
  }

  // Log sensor values to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
}

void handleRoot() {
  String html = "<html><head>";
  html += "<title>ESP8266 Web Server</title>";
  html += "<script>";
  html += "function updateSensors() {";
  html += "fetch('/sensorData').then(response => response.json()).then(data => {";
  html += "document.getElementById('temp').innerText = data.temperature + ' °C';";
  html += "document.getElementById('humidity').innerText = data.humidity + ' %';";
  html += "document.getElementById('distance').innerText = data.distance + ' cm';";
  html += "}).catch(err => console.log(err));";
  html += "}";
  html += "setInterval(updateSensors, 5000);"; // Update every 5 seconds
  html += "</script></head><body>";
  html += "<h1>ESP8266 Web Server</h1>";
  html += "<h2>Sensor Data</h2>";
  html += "<p>Temperature: <span id='temp'>Loading...</span></p>";
  html += "<p>Humidity: <span id='humidity'>Loading...</span></p>";
  html += "<p>Distance: <span id='distance'>Loading...</span></p>";
  html += "<h2>LED Controls</h2>";
  html += "<p><a href='/led1/on'><button>Turn LED 1 ON</button></a>";
  html += "<a href='/led1/off'><button>Turn LED 1 OFF</button></a></p>";
  html += "<p><a href='/led2/on'><button>Turn LED 2 ON</button></a>";
  html += "<a href='/led2/off'><button>Turn LED 2 OFF</button></a></p>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void sendSensorData() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"distance\":" + String(distance);
  json += "}";
  server.send(200, "application/json", json);
}
