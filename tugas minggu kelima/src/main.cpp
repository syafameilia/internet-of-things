#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

// Deklarasi fungsi
void updateDisplay();
void updateWeatherData();

const char* ssid = "Wokwi-GUEST";
const char* password = "";
String apiKey = "20ca0ff523294dcdeb424dfc5802e21b";
String city = "Malang";
String units = "metric";
String server = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&units=" + units + "&appid=" + apiKey;

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Tombol
const int tombolKanan = 13;
const int tombolKiri = 12;

// Data cuaca
String suhu = "---";
String deskripsi = "---";
String kelembaban = "---";
String kecepatanAngin = "---";

// Navigasi halaman
int halamanAktif = 0;
const int jumlahHalaman = 4;
unsigned long waktuTerakhirTombol = 0;
const unsigned long jedaDebounce = 200;

void setup() {
  Serial.begin(115200);
  
  // Inisialisasi LCD
  lcd.init();
  lcd.backlight();
  
  // Inisialisasi tombol
  pinMode(tombolKanan, INPUT_PULLUP);
  pinMode(tombolKiri, INPUT_PULLUP);
  
  lcd.setCursor(0, 0);
  lcd.print("Weather Info:");
  
  // Koneksi WiFi
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 1);
  lcd.print("Connecting...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected!");
  delay(2000);
  lcd.clear();
  
  // Ambil data cuaca pertama kali
  updateWeatherData();
}

void loop() {
  // Cek tombol kanan (next page)
  if (digitalRead(tombolKanan) == LOW && millis() - waktuTerakhirTombol > jedaDebounce) {
    halamanAktif = (halamanAktif + 1) % jumlahHalaman;
    waktuTerakhirTombol = millis();
    updateDisplay();
  }
  
  // Cek tombol kiri (previous page)
  if (digitalRead(tombolKiri) == LOW && millis() - waktuTerakhirTombol > jedaDebounce) {
    halamanAktif = (halamanAktif - 1 + jumlahHalaman) % jumlahHalaman;
    waktuTerakhirTombol = millis();
    updateDisplay();
  }
  
  // Update data cuaca setiap menit
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 60000 || lastUpdate == 0) {
    updateWeatherData();
    lastUpdate = millis();
  }
}

void updateWeatherData() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(server);
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      
      // Parse suhu
      int tempIndex = payload.indexOf("\"temp\":");
      if (tempIndex > 0) {
        suhu = payload.substring(tempIndex + 7, payload.indexOf(",", tempIndex));
      }
      
      // Parse deskripsi
      int descIndex = payload.indexOf("\"description\":");
      if (descIndex > 0) {
        deskripsi = payload.substring(descIndex + 14, payload.indexOf("\"", descIndex + 14));
      }
      
      // Parse kelembaban
      int humIndex = payload.indexOf("\"humidity\":");
      if (humIndex > 0) {
        kelembaban = payload.substring(humIndex + 11, payload.indexOf(",", humIndex));
      }
      
      // Parse kecepatan angin
      int windIndex = payload.indexOf("\"speed\":");
      if (windIndex > 0) {
        kecepatanAngin = payload.substring(windIndex + 8, payload.indexOf(",", windIndex));
      }
      
      updateDisplay();
    } else {
      Serial.println("Error on HTTP request");
    }
    http.end();
  }
}

void updateDisplay() {
  lcd.clear();
  
  switch (halamanAktif) {
    case 0: // Suhu
      lcd.setCursor(0, 0);
      lcd.print("Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(suhu);
      lcd.print(" C");
      break;
      
    case 1: // Deskripsi
      lcd.setCursor(0, 0);
      lcd.print("Weather:");
      lcd.setCursor(0, 1);
      lcd.print(deskripsi);
      break;
      
    case 2: // Kelembaban
      lcd.setCursor(0, 0);
      lcd.print("Humidity:");
      lcd.setCursor(0, 1);
      lcd.print(kelembaban);
      lcd.print(" %");
      break;
      
    case 3: // Kecepatan angin
      lcd.setCursor(0, 0);
      lcd.print("Wind Speed:");
      lcd.setCursor(0, 1);
      lcd.print(kecepatanAngin);
      lcd.print(" m/s");
      break;
  }
}