#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DS18B20.h>

#include "secrets.h"
#include "main.h"

#define PIN_TEMP_1 4

DS18B20 ds(PIN_TEMP_1);

/* ------------------------------------------- */
/* INI WIFI                                    */
/* ------------------------------------------- */

char ssid[] = WIFI_SSID;
char pass[] = WIFI_PASSWORD;
int status = WL_IDLE_STATUS;

WiFiClient espClient;

/* ------------------------------------------- */
/* INI MQTT                                    */
/* ------------------------------------------- */

PubSubClient mqtt(espClient);

void setup() {
  Serial.begin(9600);
  Serial.print("Devices: ");
  Serial.println(ds.getNumberOfDevices());
  Serial.println();

  setup_wifi();
  mqtt.setServer(MQTT_SERVER, MQTT_SERVERPORT);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  if (!mqtt.connected()) {
    reconnect();
  }
  mqtt.loop();

  while (ds.selectNext()) {
    uint8_t address[8];
    ds.getAddress(address);

    String addressStr = "";
    for (uint8_t i = 0; i < 8; i++) {
      addressStr.concat(address[i]);
    }

    char topic[60];   
    sprintf(topic, "%s%s", "temp2mqtt/", addressStr.c_str());

    char temp[6]; 
    dtostrf(ds.getTempC(), 4, 2, temp);

    mqtt.publish(topic, temp);

    delay(1000);
  }
}


void reconnect() {
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt.connect(MQTT_CID, MQTT_USERNAME, MQTT_KEY)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");

      delay(5000);
    }
  }
}

