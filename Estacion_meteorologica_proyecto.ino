#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// Definición de pines
#define DHTPIN 2           // Pin donde está conectado el DHT22
#define LDR_PIN A0         // Pin donde está conectado el LDR.
#define LED_PIN 7          // Pin donde está conectado el LED.
#define BUZZER_PIN 8       // Pin donde está conectado el buzzer.

// Inicialización de sensores
DHT dht(DHTPIN, DHT22);         // Inicializa el DHT22
Adafruit_BMP280 bmp;            // Inicializa el BMP280
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inicializa el LCD I2C

int umbralLDR = 500;            // Umbral para detectar baja luz (ajustar según prueba)

void setup() {
  Serial.begin(9600);           // Inicializa la comunicación serial
  dht.begin();                  // Inicializa el DHT22
  bmp.begin();                  // Inicializa el BMP280
  lcd.begin(16, 2);             // Inicializa el LCD con 16 columnas y 2 filas
  lcd.backlight();              // Enciende la luz de fondo del LCD
  pinMode(LED_PIN, OUTPUT);     // Configura el pin del LED como salida
  pinMode(BUZZER_PIN, OUTPUT);  // Configura el pin del buzzer como salida
}

void loop() {
  // Lectura del DHT22
  float h = dht.readHumidity(); // Lee la humedad
  float t = dht.readTemperature(); // Lee la temperatura en Celsius

  // Lectura del BMP280
  float p = bmp.readPressure() / 100.0F; // Lee la presión en hPa

  // Lectura del LDR
  int valorLDR = analogRead(LDR_PIN); // Lee el valor del LDR

  // Mostrar datos en el LCD
  lcd.setCursor(0, 0);
  lcd.print("Temp: ");
  lcd.print(t);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Humed: ");
  lcd.print(h);
  lcd.print("%");

  // Comprobar condiciones para LED y Buzzer
  if (valorLDR < umbralLDR) { // Si el valor del LDR es menor que el umbral
    digitalWrite(LED_PIN, HIGH); // Enciende el LED
    digitalWrite(BUZZER_PIN, HIGH); // Activa el buzzer
    Serial.println("LLUVIA DETECTADA");
  } else {
    digitalWrite(LED_PIN, LOW); // Apaga el LED
    digitalWrite(BUZZER_PIN, LOW); // Desactiva el buzzer
    Serial.println("SIN LLUVIA");
  }

  // Imprimir datos en el monitor serial
  Serial.print("Humedad: "); Serial.print(h);
  Serial.print("%, Temperatura: "); Serial.print(t);
  Serial.print("C, Presión: "); Serial.print(p);
  Serial.print(" hPa, LDR Value: "); Serial.println(valorLDR);

  delay(2000); // Espera 2 segundos antes de la siguiente lectura
}
