#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

/* -------- WiFi -------- */
const char* ssid1 = "Sarvadnya";
const char* pass1 = "air14535";
const char* ssid2 = "chimcham";
const char* pass2 = "123456789";

/* -------- HiveMQ Cloud -------- */
const char* mqttServer = "4d679eb8c29c4e029130d4045c0c1a2e.s1.eu.hivemq.cloud";
const int   mqttPort   = 8883;
const char* mqttUser   = "neerdrishti";
const char* mqttPass   = "Neerdrishti123";
const char* topic      = "neerdrishti/sonar/map";

/* -------- Sensors -------- */
#define TF 5
#define EF 18
#define TL 17
#define EL 16
#define TR 4
#define ER 2

WiFiClientSecure secureClient;
PubSubClient mqtt(secureClient);

/* -------- Distance -------- */
long readCM(int t, int e){
  digitalWrite(t, LOW); delayMicroseconds(2);
  digitalWrite(t, HIGH); delayMicroseconds(10);
  digitalWrite(t, LOW);
  long d = pulseIn(e, HIGH, 30000);
  if (d == 0) return -1;
  return d / 58;
}

void connectWiFi(){
  WiFi.begin(ssid1, pass1);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 8000) delay(500);
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid2, pass2);
    while (WiFi.status() != WL_CONNECTED) delay(500);
  }
}

void reconnectMQTT(){
  while (!mqtt.connected()) {
    mqtt.connect("NeerdrishtiESP32", mqttUser, mqttPass);
    delay(2000);
  }
}

void setup(){
  Serial.begin(115200);

  pinMode(TF, OUTPUT); pinMode(EF, INPUT);
  pinMode(TL, OUTPUT); pinMode(EL, INPUT);
  pinMode(TR, OUTPUT); pinMode(ER, INPUT);

  connectWiFi();

  secureClient.setInsecure();   // OK for HiveMQ Cloud
  mqtt.setServer(mqttServer, mqttPort);
}

void loop(){
  if (!mqtt.connected()) reconnectMQTT();
  mqtt.loop();

  long f = readCM(TF, EF);
  long l = readCM(TL, EL);
  long r = readCM(TR, ER);

  if (f > 0 && l > 0 && r > 0) {
    String payload = "{";
    payload += "\"front\":" + String(f) + ",";
    payload += "\"left\":"  + String(l) + ",";
    payload += "\"right\":" + String(r);
    payload += "}";

    mqtt.publish(topic, payload.c_str());
    Serial.println(payload);
  }

  delay(150);
}
