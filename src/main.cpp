#include <SPI.h>
#include <WiFiNINA.h>
#include <MQTT.h>
#include <MQTTClient.h>

#include "secrets.h"
#include "itech_dxl.h"

const char BROKER_ADDR[] = "broker.hivemq.com";
const char SUB_TOPIC[] = "ITECH_COM_WS/robot/cmd";
const char PUB_TOPIC[] = "ITECH_COM_WS/robot/status";

WiFiClient wifi;
MQTTClient client;
uint64_t lastMillis = 0;

void onMessage(String &topic, String &payload){
    Serial.println(
        "Hey I received this: " + 
        topic +
        "| " +
        payload
    );
    if (payload=="ON"){
        digitalWrite(LED_BUILTIN,HIGH);
    }
    else if (payload == "OFF")
    {
        digitalWrite(LED_BUILTIN,LOW);
    }
    
}

void onMessageAdvanced(MQTTClient *client, char topic[], char bytes[], int length){
    String topic_str(topic);
    Serial.print("incoming: "+topic_str+" ("+String(length)+") ");
    if (length==8){
        int32_t target_left = static_cast<int32_t>(
            static_cast<uint8_t>(bytes[3]) << 24 |
            static_cast<uint8_t>(bytes[2]) << 16 |
            static_cast<uint8_t>(bytes[1]) << 8 |
            static_cast<uint8_t>(bytes[0])
        );
        int32_t target_right = -static_cast<int32_t>(
            static_cast<uint8_t>(bytes[7]) << 24 |
            static_cast<uint8_t>(bytes[6]) << 16 |
            static_cast<uint8_t>(bytes[5]) << 8 |
            static_cast<uint8_t>(bytes[4])
        );
        Serial.print("left :"+String(target_left) + "| right"+String(target_right));
        SetNewTarget(target_left,target_right);
    }
    Serial.println();
}



void connect(){
    Serial.print("checking wifi");
    Serial.print("...");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(" success!");
    String mqtt_client_id = String("myclient-"+String(random(UINT16_MAX)));
    Serial.print("connecting with id "+mqtt_client_id);
    Serial.print("...");
    while (!client.connect(mqtt_client_id.c_str()));
    {
        Serial.print('.');
        delay(1000);
    }
    Serial.println(" success!");
    client.subscribe(SUB_TOPIC);
}

void setup() {
    pinMode(LED_BUILTIN,OUTPUT);
    randomSeed(analogRead(0));
    Serial.begin(9600);

    SetupDxl();
    WiFi.begin(WIFI_SSID,WIFI_PSWD);
    client.begin(BROKER_ADDR,wifi);
    //client.onMessage(onMessage);
    client.onMessageAdvanced(onMessageAdvanced);
    connect();
    Serial.println("Init done!");
}

void loop() {
    client.loop();
    if (!client.connected()){
        connect();
    }
    SetAndExecute();
    if (millis() - lastMillis > 1000){
        String robot_info = GetRobotInfo();
        client.publish(PUB_TOPIC,robot_info);
        lastMillis = millis();
    }
}