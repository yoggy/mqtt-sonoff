//
// mqtt-sonoff.ino - a relay control sample sketch using MQTT.
//
// SONOFF
//   https://www.itead.cc/sonoff-wifi-wireless-switch-1.html
//
// pubsubclient
//   https://github.com/knolleary/pubsubclient/
//
// How to use:
//
//     $ git clone https://github.com/yoggy/mqtt-sonoff
//     $ cd mqtt-sonoff
//     $ cp config.ino.sample config.ino
//     $ vi config.ino
//       - edit wifi_ssid, wifi_password, mqtt_server, mqtt_subscribe_topic, ... etc
//     $ open mqtt-sonoff.ino
//
//  Arduino settings;
//    * Board: Generic ESP8266 Module
//    * Flash Mode: DIO
//    * Flash Frequency: 40MHz
//    * Upload Using: Serial
//    * CPU Frequency: 80MHz
//    * Flash Size: 1M (64K SPIFFS)
//    * Debug Port: Disabled
//    * Debug Level: None
//    * Reset Method: ck
//    * Upload Speed: 115200
//
// license:
//     Copyright (c) 2017 yoggy <yoggy0@gmail.com>
//     Released under the MIT license
//     http://opensource.org/licenses/mit-license.php;
//
#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/

// Wif config
extern char *wifi_ssid;
extern char *wifi_password;
extern char *mqtt_server;
extern int  mqtt_port;

extern char *mqtt_client_id;
extern bool mqtt_use_auth;
extern char *mqtt_username;
extern char *mqtt_password;

extern char *mqtt_subscribe_topic;

WiFiClient wifi_client;
void mqtt_sub_callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, mqtt_sub_callback, wifi_client);

void LED_ON() {
  digitalWrite(13, LOW);
}

void LED_OFF() {
  digitalWrite(13, HIGH);
}

static bool enable_relay = false;

void RELAY_ON() {
  digitalWrite(12, HIGH);
  enable_relay = true;
  Serial.println("RELAY_ON");
}

void RELAY_OFF() {
  digitalWrite(12, LOW);
  enable_relay = false;
  Serial.println("RELAY_OFF");
}

void RELAY_TOGGLE() {
  if (enable_relay == true) {
    RELAY_OFF();
  }
  else {
    RELAY_ON();
  }
}

void setup() {
  Serial.begin(9600);
  
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(0, INPUT);

  WiFi.begin(wifi_ssid, wifi_password);
  WiFi.mode(WIFI_STA);
  int wifi_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    if (wifi_count % 2 == 0) {
      LED_ON();
    }
    else {
      LED_OFF();
    }
    wifi_count ++;
    delay(500);
  }

  bool rv = false;
  if (mqtt_use_auth == true) {
    rv = mqtt_client.connect(mqtt_client_id, mqtt_username, mqtt_password);
  }
  else {
    rv = mqtt_client.connect(mqtt_client_id);
  }
  if (rv == false) {
    reboot();
  }

  LED_ON();
  mqtt_client.subscribe(mqtt_subscribe_topic);
}

void reboot() {
  for (int i = 0; i < 10; ++i) {
    LED_ON();
    delay(100);
    LED_OFF();
    delay(100);
  };

  ESP.restart();

  while (true) {
    LED_ON();
    delay(100);
    LED_OFF();
    delay(100);
  };
}

void loop() {
  if (!mqtt_client.connected()) {
    reboot();
  }
  mqtt_client.loop();

  check_button_status();
}

void mqtt_sub_callback(char* topic, byte* payload, unsigned int length) {
  LED_OFF();
  delay(50);
  LED_ON();

  if (length >= 16) return;

  char cmd[16];
  memset(cmd, 0, 16);
  memcpy(cmd, payload, length);
  Serial.print("recv_cmd=");
  Serial.println(cmd);

  if (strcmp(cmd, "on") == 0) {
    RELAY_ON();
  }
  else if (strcmp(cmd, "off") == 0) {
    RELAY_OFF();
  }
  else if (strcmp(cmd, "toggle") == 0) {
    RELAY_TOGGLE();
  }
}

int count = 100;
void check_button_status()
{
  if (digitalRead(0) == LOW) {
    if (count > 0) {
      count --;
      if (count == 0) {
        LED_OFF();
        delay(50);
        LED_ON();
        RELAY_TOGGLE();
      }
    }
  }
  else {
    count = 100;
  }
}

