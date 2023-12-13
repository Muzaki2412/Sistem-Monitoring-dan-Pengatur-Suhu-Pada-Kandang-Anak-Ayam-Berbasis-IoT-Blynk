#define BLYNK_TEMPLATE_ID "TMPL6MZhBhVUq"
#define BLYNK_TEMPLATE_NAME "Monitoring Kandang Ayam"
#define BLYNK_AUTH_TOKEN "nm-RsIQv0Cspl_5jkuJMJAyBuwHCguGa"
#include <DHT.h>
#include <BlynkSimpleEsp32.h>

#define DHTPIN 4
#define RELAY_PIN1 21
#define RELAY_PIN2 22
#define LED_INDICATOR_PIN 2  // Pin untuk LED indikator

#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

// Tambahkan variabel boolean untuk menandai status on/off manual
bool manualControl = false;
unsigned long lastManualChangeTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(LED_INDICATOR_PIN, OUTPUT);
  digitalWrite(RELAY_PIN1, LOW);
  digitalWrite(RELAY_PIN2, LOW);
  digitalWrite(LED_INDICATOR_PIN, LOW);

  dht.begin();

  WiFi.begin("PITIK", "12345678");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("WiFi Connected");
  Blynk.begin(BLYNK_AUTH_TOKEN, "PITIK", "12345678");
  Serial.println("Blynk Connected");
  // Setup Blynk buttons
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);
}

BLYNK_WRITE(V3){
  int value = param.asInt();
  if (value == 1) {
    // "on" button pressed, turn on the light
    digitalWrite(RELAY_PIN2, LOW);
    Serial.println("lampu on");
    // Matikan kipas jika sedang hidup dalam mode otomatis
    digitalWrite(RELAY_PIN1, HIGH);
    Serial.println("kipas off");
    // Aktifkan status manual control
    manualControl = true;
    // Perbarui timestamp perubahan mode manual
    lastManualChangeTime = millis();
    // Matikan LED indikator
    digitalWrite(LED_INDICATOR_PIN, LOW);
  } else {
    // "off" button pressed, turn off the light
    digitalWrite(RELAY_PIN2, HIGH);
    Serial.println("lampu off");
    // Nonaktifkan status manual control
    manualControl = false;
    // Perbarui timestamp perubahan mode manual
    lastManualChangeTime = millis();
    // Matikan LED indikator jika kipas tidak aktif
    if (digitalRead(RELAY_PIN1) == HIGH) {
      digitalWrite(LED_INDICATOR_PIN, LOW);
    }
  }
}


BLYNK_WRITE(V4){
  int value = param.asInt();
  if (value == 1) {
    // "on" button pressed, turn on the fan
    digitalWrite(RELAY_PIN1, LOW);
    Serial.println("kipas on");
    // Matikan lampu jika sedang hidup dalam mode otomatis
    digitalWrite(RELAY_PIN2, HIGH);
    Serial.println("lampu off");
    // Aktifkan status manual control
    manualControl = true;
    // Perbarui timestamp perubahan mode manual
    lastManualChangeTime = millis();
    // Nyalakan LED indikator
    digitalWrite(LED_INDICATOR_PIN, HIGH);
  }  else {
    // "off" button pressed, turn off the fan
    digitalWrite(RELAY_PIN1, HIGH);
    Serial.println("kipas off");
    // Nonaktifkan status manual control
    manualControl = false;
    // Perbarui timestamp perubahan mode manual
    lastManualChangeTime = millis();
    // Matikan LED indikator
    digitalWrite(LED_INDICATOR_PIN, LOW);
  }
}

void loop() {
  Blynk.run();
  
  // Jika tidak dalam mode manual control, baca sensor
  if (!manualControl) {
    unsigned long currentTime = millis();
    // Cek apakah sudah 5 detik sejak perubahan mode manual terakhir
    if (currentTime - lastManualChangeTime >= 5000) {
      readDHT();
    }
  }
  
  delay(500);
}

void readDHT() {
  float suhu = dht.readTemperature();
  float kelembaban = dht.readHumidity();

  if (!isnan(suhu) && !isnan(kelembaban)) {
    Serial.print("Suhu: ");
    Serial.print(suhu);
    Serial.print("°C, Kelembaban: ");
    Serial.print(kelembaban);
    Serial.println("%");
  }

  // Your existing logic for relay control
  if (suhu > 27.00) {
    digitalWrite(RELAY_PIN1, LOW);
    Serial.println("kipas on");
    // Nyalakan LED indikator
    digitalWrite(LED_INDICATOR_PIN, HIGH);
  } else {
    digitalWrite(RELAY_PIN1, HIGH);
    Serial.println("kipas off");
    // Matikan LED indikator
    digitalWrite(LED_INDICATOR_PIN, LOW);
  }

  if (suhu < 26.00) {
    digitalWrite(RELAY_PIN2, LOW);
    Serial.println("lampu on");
  } else {
    digitalWrite(RELAY_PIN2, HIGH);
    Serial.println("lampu off");
  }

  // Send temperature and humidity values to Blynk
  Blynk.virtualWrite(V1, suhu);
  Blynk.virtualWrite(V2, kelembaban);

  Serial.println("suhu: " + String(suhu) + " °C");
  Serial.println("kelembaban: " + String(kelembaban) + "%");
}
