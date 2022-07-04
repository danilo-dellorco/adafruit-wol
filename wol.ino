#include <ESP8266WiFi.h>
#include <WakeOnLan.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "config.h"

WiFiUDP UDP;
WakeOnLan WOL(UDP);

#define FEED_NAME "WOL"
#define DESKTOP_MAC "70:4D:7B:B2:23:0B"
#define LED_RED 4
#define LED_GREEN 5
#define LED_BLUE 16

AdafruitIO_Feed *feed = io.feed(FEED_NAME);
boolean disconnected = false;

void connect_wifi() {
    WiFi.mode(WIFI_STA);
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print('.');
        delay(500);
    }
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_BLUE, HIGH);
    Serial.println("Connected to WiFi");
}

void connect_adafruit() {
    io.connect();
    Serial.print("Connecting to Adafruit IO");
    while (io.status() < AIO_CONNECTED) {
        Serial.print(".");
        delay(500);
    }
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, HIGH);
    Serial.println(io.statusText());
}

void setup() {
    pinMode(LED_RED, OUTPUT);
    pinMode(LED_GREEN, OUTPUT);
    pinMode(LED_BLUE, OUTPUT);
    Serial.begin(115200);

    connect_wifi();
    WOL.setRepeat(3, 100);
    WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());
    connect_adafruit();
}

void loop() {
    io.run();
    feed->onMessage(handleMessage);
    if (io.status() < AIO_CONNECTED) {
        Serial.println("WiFi Disconnected!");
        digitalWrite(LED_RED, HIGH);
        digitalWrite(LED_GREEN, LOW);
        while (io.status() < AIO_CONNECTED) {
            Serial.println("Waiting for Reconnection...");
            io.connect();
            delay(5000);
        }
        digitalWrite(LED_RED, LOW);
        digitalWrite(LED_GREEN, HIGH);
        Serial.println("WiFi Connection Restored!");
    }
}

void blink_led(int blink_color, int old_color) {
    digitalWrite(old_color, LOW);
    for (int i = 0; i < 3; i++) {
        digitalWrite(blink_color, HIGH);
        delay(50);
        digitalWrite(blink_color, LOW);
        delay(50);
    }
    digitalWrite(old_color, HIGH);
}

void handleMessage(AdafruitIO_Data *data) {
    Serial.println(data->toString());
    if (data->toString() == "wake") {
        Serial.print("Sending Magic Packet...");
        WOL.sendMagicPacket(DESKTOP_MAC);
        blink_led(LED_BLUE, LED_GREEN);
    }

    if (data->toString() == "ping") {
        blink_led(LED_RED, LED_GREEN);
        feed->save("working");
    }
}
