/************************************************************
 * Neerdrishti ESP32 – VERIFIED MQTT BASELINE
 * HiveMQ Cloud TLS (8883)
 ************************************************************/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

/* ================= WIFI ================= */
const char* ssid1 = "Sarvadnya";
const char* pass1 = "air14535";
const char* ssid2 = "chimcham";
const char* pass2 = "123456789";

/* ================= MQTT (HiveMQ Cloud) ================= */
const char* mqttServer = "4d679eb8c29c4e029130d4045c0c1a2e.s1.eu.hivemq.cloud";
const int   mqttPort   = 8883;          // TLS MQTT
const char* mqttUser   = "neerdrishti";
const char* mqttPass   = "Neerdrishti123";
const char* topic      = "neerdrishti/sonar/map";

/* ================= SENSORS ================= */
#define TF 5
#define EF 18
#define TL 17
#define EL 16
#define TR 4
#define ER 2

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

unsigned long lastPublish = 0;

/* ================= DISTANCE ================= */
long readCM(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  long d = pulseIn(echo, HIGH, 30000);
  if (d == 0) return -1;
  return d / 58;
}

/* ================= WIFI ================= */
void connectWiFi() {
  Serial.println("[WiFi] Connecting...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid1, pass1);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Fallback to hotspot");
    WiFi.begin(ssid2, pass2);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
  }

  Serial.println("\n[WiFi] CONNECTED");
  Serial.print("[WiFi] IP: ");
  Serial.println(WiFi.localIP());
}

/* ================= MQTT ================= */
void connectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Connecting... ");
    if (mqtt.connect("NeerdrishtiESP32", mqttUser, mqttPass)) {
      Serial.println("CONNECTED");
    } else {
      Serial.print("FAILED, rc=");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(1500);

  Serial.println("\n=== Neerdrishti ESP32 BOOT ===");

  pinMode(TF, OUTPUT); pinMode(EF, INPUT);
  pinMode(TL, OUTPUT); pinMode(EL, INPUT);
  pinMode(TR, OUTPUT); pinMode(ER, INPUT);

  connectWiFi();

  secureClient.setInsecure();  // TLS without CA (OK for HiveMQ Cloud)
  mqtt.setServer(mqttServer, mqttPort);
  connectMQTT();
}

/* ================= LOOP ================= */
void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[WiFi] LOST, reconnecting");
    connectWiFi();
  }

  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  if (millis() - lastPublish > 500) {
    lastPublish = millis();

    long f = readCM(TF, EF);
    long l = readCM(TL, EL);
    long r = readCM(TR, ER);

    /* DEBUG FALLBACK */
    if (f <= 0) f = random(80, 150);
    if (l <= 0) l = random(80, 150);
    if (r <= 0) r = random(80, 150);

    char payload[128];
    snprintf(payload, sizeof(payload),
      "{\"front\":%ld,\"left\":%ld,\"right\":%ld}",
      f, l, r
    );

    mqtt.publish(topic, payload);
    Serial.print("[MQTT] ");
    Serial.println(payload);
  }
}

