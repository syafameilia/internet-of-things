#include <WiFi.h>
#include <PubSubClient.h>
#include <DHTesp.h>

const int LED_RED = 2;
const int DHT_PIN = 15;
DHTesp dht; 

// Update these with values suitable for your network.

const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.emqx.io";//"test.mosquitto.org";//

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
float temp = 0;
float hum = 0;

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA); // Mengatur ESP32 sebagai station (klien WiFi)
  WiFi.begin(ssid, password); // Memulai koneksi ke jaringan WiFi

  while (WiFi.status() != WL_CONNECTED) { // Tunggu sampai koneksi berhasil
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros()); // Inisialisasi seed random untuk client ID MQTT

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP()); // Tampilkan alamat IP yang didapatkan
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]); // Tampilkan pesan yang diterima
  }
  Serial.println();

  // Jika karakter pertama payload adalah '1', nyalakan LED, jika tidak matikan
  if ((char)payload[0] == '1') {
    digitalWrite(LED_RED, HIGH);
  } else {
    digitalWrite(LED_RED, LOW);
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX); // Membuat client ID unik

    if (client.connect(clientId.c_str())) {
      Serial.println("Connected");
      client.publish("IOT/Test1/mqtt", "Test IOT"); // Kirim pesan awal ke broker
      client.subscribe("IOT/Test1/mqtt"); // Subscribe ke topik tertentu
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000); // Tunggu 5 detik sebelum mencoba koneksi ulang
    }
  }
}

void setup() {
  pinMode(LED_RED, OUTPUT); // Set pin LED sebagai output
  Serial.begin(115200); // Mulai komunikasi serial dengan baud rate 115200
  setup_wifi(); // Panggil fungsi koneksi WiFi
  client.setServer(mqtt_server, 1883); // Set alamat dan port broker MQTT
  client.setCallback(callback); // Set fungsi callback untuk pesan masuk
  dht.setup(DHT_PIN, DHTesp::DHT22); // Inisialisasi sensor DHT22 pada pin yang ditentukan
}

void loop() {
  if (!client.connected()) {
    reconnect(); // Jika MQTT belum terhubung, lakukan reconnect
  }
  client.loop(); // Proses komunikasi MQTT

  unsigned long now = millis();
  if (now - lastMsg > 2000) { // Kirim data setiap 2 detik
    lastMsg = now;
    TempAndHumidity data = dht.getTempAndHumidity(); // Baca data sensor

    String temp = String(data.temperature, 2); // Konversi suhu ke string dengan 2 desimal
    client.publish("IOT/Test1/temp", temp.c_str()); // Publish suhu ke broker MQTT

    String hum = String(data.humidity, 1); // Konversi kelembaban ke string dengan 1 desimal
    client.publish("IOT/Test1/hum", hum.c_str()); // Publish kelembaban ke broker MQTT

    Serial.print("Temperature: ");
    Serial.println(temp);
    Serial.print("Humidity: ");
    Serial.println(hum);
  }
}