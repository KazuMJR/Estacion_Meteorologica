#include <WiFiS3.h>
#include <DHT.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>

// ==== CONFIGURACIÓN DE SENSORES ====
#define DHTPIN 2
#define DHTTYPE DHT22
#define SENSOR_LLUVIADIG 3
#define SENSOR_SUELO A0
#define SENSOR_GAS A1
#define LED_PIN 7
#define BUZZER_PIN 8

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ==== CONFIGURACIÓN DE RED ====
char ssid[] = "ARRIS-0852";     
char pass[] = "DCA633FC0852";
WiFiClient client;
char server[] = "192.168.0.5"; // IP de tu servidor Node.js

// ==== SETUP ====
void setup() {
  Serial.begin(9600);
  dht.begin();
  bmp.begin(0x76);
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(SENSOR_LLUVIADIG, INPUT);
  pinMode(SENSOR_SUELO, INPUT);
  pinMode(SENSOR_GAS, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");

  // Conexión WiFi
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    digitalWrite(LED_PIN, millis() % 1000 < 500 ? HIGH : LOW); // parpadeo mientras conecta
  }

  lcd.clear();
  lcd.print("WiFi conectado");
  Serial.println("\nWiFi conectado.");
  digitalWrite(LED_PIN, HIGH); // LED fijo cuando WiFi OK
  delay(2000);
}

// ==== LOOP PRINCIPAL ====
void loop() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float pres = bmp.readPressure() / 100.0;
  int lluvia = digitalRead(SENSOR_LLUVIADIG);
  int humedadSuelo = analogRead(SENSOR_SUELO);
  int gas = analogRead(SENSOR_GAS);

  String lluviaTxt = lluvia == 0 ? "SI" : "NO";

  // Mostrar datos en LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temp, 1);
  lcd.print("C H:");
  lcd.print(hum, 1);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("Pres:");
  lcd.print(pres, 1);
  lcd.print("hPa");

  // Alertas LED y Buzzer
  bool alerta = false;

  if (temp > 35) {
    tone(BUZZER_PIN, 1000, 500); // beep medio segundo
    lcd.setCursor(0, 1);
    lcd.print("ALERTA: TEMP!");
    alerta = true;
  } 
  if (gas > 600) {
    tone(BUZZER_PIN, 1500, 1000); // beep largo
    lcd.setCursor(0, 1);
    lcd.print("ALERTA: GAS!");
    alerta = true;
  }
  if (lluviaTxt == "SI") {
    tone(BUZZER_PIN, 800, 300); // beep corto
    lcd.setCursor(0, 1);
    lcd.print("ALERTA: LLUVIA!");
    alerta = true;
  }

  if (!alerta) {
    noTone(BUZZER_PIN); // apagar buzzer
    digitalWrite(LED_PIN, HIGH); // LED WiFi OK
  } else {
    digitalWrite(LED_PIN, millis() % 500 < 250 ? HIGH : LOW); // parpadeo LED alerta
  }

  // Envío de datos JSON
  StaticJsonDocument<256> json;
  json["temperatura"] = temp;
  json["humedad"] = hum;
  json["presion"] = pres;
  json["lluvia"] = lluviaTxt;
  json["humedadSuelo"] = humedadSuelo;
  json["gas"] = gas;

  String payload;
  serializeJson(json, payload);

  if (client.connect(server, 3000)) {
    client.println("POST /api/lecturas HTTP/1.1");
    client.println("Host: servidor");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(payload.length());
    client.println();
    client.print(payload);
    client.stop();
    Serial.println("Datos enviados:");
    Serial.println(payload);
  } else {
    Serial.println("Error al conectar al servidor");
  }

  delay(10000); // cada 10 segundos
}
