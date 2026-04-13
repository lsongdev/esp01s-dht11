#include <Arduino.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define DHTPIN 2
#define DHTTYPE DHT11

#define WIFI_SSID "wifi@lsong.org"
#define WIFI_PASS "song940@163.com"

#define MQTT_SERVER "broker.emqx.io"
#define MQTT_PORT 1883
#define MQTT_TOPIC "yim-temp"
#define MQTT_CLIENT_ID "esp01-dht11-%06X"

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectWiFi()
{
  Serial.println();
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT()
{
  char clientId[32];
  snprintf(clientId, sizeof(clientId), MQTT_CLIENT_ID, ESP.getChipId());
  while (!mqtt.connected())
  {
    Serial.print("Connecting to MQTT...");
    if (mqtt.connect(clientId))
    {
      Serial.println("connected");
    }
    else
    {
      Serial.printf("failed, rc=%d, retry in 2s\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setBufferSize(256);

  connectWiFi();
  connectMQTT();

  delay(1000);
  Serial.println("DHT11 + MQTT started");
}

void loop()
{
  if (!mqtt.connected())
  {
    connectMQTT();
  }
  mqtt.loop();

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char payload[64];
  snprintf(payload, sizeof(payload), "{\"temperature\":%.1f,\"humidity\":%.1f}", temperature, humidity);

  Serial.printf("Publishing: %s\n", payload);
  mqtt.publish(MQTT_TOPIC, payload);

  delay(2000);
}