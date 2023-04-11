#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>
extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#include <AsyncMqttClient.h>
#include <HTTPClient.h>

//Declaramos variables y constantes
#define WIFI_SSID "Wi-Fi Unimagdalena"
#define MQTT_HOST IPAddress( 3, 86, 13, 130)
#define MQTT_PORT 1883
#define MQTT_PUB_TEMP "Temperatura"
#define MQTT_PUB_HUM "Humedad"
#define MQTT_PUB_ST "Sensacion Termica"

Adafruit_BME280 bme;
int ventilador = 4;
int foco = 5;
float temp;
float hum;
float temp_F;
float st;
String apiKey = "4734500";              
String phone_number = "+573007421693"; 
String mensaje;
String url;   
AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
       

//Funcion conectar Wi-Fi
void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID);
}

//Funcion conectar MQTT
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

//Funcion Eventos Wi-Fi
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0); 
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

//Funcion Conexion MQTT
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

//Funcion Desconexion MQTT
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) {
    xTimerStart(mqttReconnectTimer, 0);
  }
}

//Funcion para mostar cuando se publica un mensaje
void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

// Funcion que se encarga de inicializar las conexiones Wi-Fi y MQTT. Configuracion de los temporizadores para la reconexion Wi-Fi y MQTT.
void setup() {

  pinMode(ventilador, OUTPUT);
  pinMode(foco, OUTPUT); 
  Serial.begin(115200);
  Serial.println();
   
  if (!bme.begin(0x76)) {
   
    while (1);
  }
  
  mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  mqttClient.setCredentials("guest", "guest");
  connectToWifi();
   
  
}

// Funcion para leer la temperatura, humedad, sensacion termica. Tambien sirve para hacer las publicaciones en los topicos MQTT
void loop() {
  
  temp = bme.readTemperature();
  temp_F = 1.8 * bme.readTemperature() + 32;
  hum = bme.readHumidity();
  st = ((-42.379 + 2.04901523 * temp_F + 10.14333127 * hum + -0.22475541 * temp_F * hum + -0.00683783 * pow(temp_F, 2) + -0.05481717 * pow(hum, 2) + 0.00122874 * pow(temp_F, 2) * hum + 0.00085282 * temp_F * pow(hum, 2) + -0.00000199 * pow(temp_F, 2) * pow(hum, 2)) - 32) / 1.8;
  
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                         
  uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_ST, 1, true, String(st).c_str());
  
  
  message_to_whatsapp();
  simulacion();
  
  
  
  delay(1000);                            
    
}






void simulacion () {
  if(temp<=29.6){
    digitalWrite(ventilador, HIGH);    
    digitalWrite(foco, LOW);
  }

 if(temp>=30.6){
    digitalWrite(ventilador, LOW);    
    digitalWrite(foco, HIGH);
  }
    
}




// Funcion para enviar mensajes via Whatsapp
void message_to_whatsapp() {
  switch (true) {
    case (1):
      if (temp < 29.5 || temp > 30.7) {
        mensaje = "Cuidado, la Temperatura esta fuera de los limites establecidos.";
        url = "https://api.callmebot.com/whatsapp.php?phone=" + phone_number + "&apikey=" + apiKey + "&text=" + urlencode(mensaje);
        HTTPClient http;
        http.begin(url);
        httpCode = http.POST(url);
        http.end();
        mensaje_enviado_temp = true;
      }
      break;
    case (2):
      if (hum <= 40 || hum >= 60) {
        mensaje = "Cuidado, la Humedad esta fuera de los limites establecidos.";
        url = "https://api.callmebot.com/whatsapp.php?phone=" + phone_number + "&apikey=" + apiKey + "&text=" + urlencode(mensaje);
        HTTPClient http;
        http.begin(url);
        httpCode = http.POST(url);
        http.end();
        mensaje_enviado_hum = true;
      }
      break;
    default:
      break;
  }
}

// Funcion para codificar la cadena de caracteres en formato URL antes de enviarla en la solicitud HTTP
String urlencode(String str)  
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        
      }
      yield();
    }
    return encodedString;
}