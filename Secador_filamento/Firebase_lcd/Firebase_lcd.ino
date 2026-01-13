   #include <WiFi.h>
#include <FirebaseESP32.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -------- WIFI --------
#define WIFI_SSID "INFINITUM60B2"
#define WIFI_PASSWORD "HAqma9G3vP"

// -------- FIREBASE --------
#define FIREBASE_HOST "rtos2mm7-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "p1SJLi0YZ5ecMIJ9w1VfOlqCc5nflQ1ugUvmjQe1"

// -------- DHT11 --------
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// -------- LCD --------
LiquidCrystal_I2C lcd(0x27, 16, 2);

// -------- FIREBASE --------
FirebaseData fbdo;

void setup() {
  Serial.begin(115200);

  // LCD
  Wire.begin(21,22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Conectando...");

  // DHT
  dht.begin();

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.print("WiFi Conectado");

  // FIREBASE
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  delay(2000);
}

void loop() {

  float temp = dht.readTemperature();
  float hum  = dht.readHumidity();

  if(isnan(temp) || isnan(hum)){
    Serial.println("Error leyendo DHT11");
    return;
  }

  // -------- LCD --------
  lcd.setCursor(0,0);
  lcd.print("Temp: ");
  lcd.print(temp,1);
  lcd.print(" C   ");

  lcd.setCursor(0,1);
  lcd.print("Hum:  ");
  lcd.print(hum,1);
  lcd.print(" %   ");

  // -------- FIREBASE --------
  Firebase.setFloat(fbdo, "/sensor/temperature", temp);
  Firebase.setFloat(fbdo, "/sensor/humidity", hum);

  Serial.printf("Temp: %.1f C | Hum: %.1f %%\n", temp, hum);

  delay(5000);
}

