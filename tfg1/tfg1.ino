//PLACA DE MIGUE--> ESP_3097139
//-----------------------------------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHTesp.h"
#include <ArduinoJson.h>
//-----------------------------------------------------
DHTesp dht;
ADC_MODE(ADC_VCC);

struct registro_datos {
        float temperatura;
        float humedad;
        };

WiFiClient wClient;
PubSubClient mqtt_client(wClient);
//-----------------------------------------------------
// Introducimos los valores de nuestra red WiFi
const char* ssid =  "huerticawifi"; //"iPhone";//"DIGIFIBRA-f2CX";
const char* password = "4cc3sshu3rt1c4"; //"aaaaaaaa";//"ekhXP6XD4S";
const char* mqtt_server = "iot.ac.uma.es";
//-----------------------------------------------------
// Introducimos los valores del servidor MQTT
const char* mqtt_user = "infind";//"II3";
const char* mqtt_pass = "zancudo";//"79qUYsdM";
//-----------------------------------------------------
// La placa que vamos a utilizar en el proyecto es ESP_3097139
char ID_PLACA[16];
//-----------------------------------------------------
// Esta funcion va a ser la que usemos para que se establezca la conexion WiFi
void conecta_wifi() {
  Serial.printf("\nConnecting to %s:\n", ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(200);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected, IP address: %s\n", WiFi.localIP().toString().c_str());
}

//-----------------------------------------------------

// Esta funcion va a ser la que usemos para que se establezca la conexion MQTT
  void conecta_mqtt() {
  char cadenaConectado[128];  // cadena de 128 caracteres 
  char cadenaDesconectado[128];  // cadena de 128 caracteres 
  // A continuación, creamos una cadena de caracteres de 128 de longitud donde guardaremos 
  // el mensaje de ultima voluntad indicando que nos hemos desconectado repentinamente
  snprintf(cadenaConectado, 128,"{\"online\": false}");
  snprintf(cadenaDesconectado, 128,"{\"online\": true}");
  // Mienrtas que no establezcamos la conexion no saldremos del bucle
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(ID_PLACA,mqtt_user,mqtt_pass/*,"II3/ESP3097139/conexion",2,true,cadenaDesconectado*/)) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.publish("II3/ESP3097139/conexion",cadenacONECTADO);
    } else {
      Serial.printf("failed, rc=%d  try again in 5s\n", mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-----------------------------------------------------
//     SETUP
//-----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empieza setup...");
  dht.setup(5, DHTesp::DHT22); // Connect DHT22 sensor to GPIO 5
  // crea topics usando id de la placa
  sprintf(ID_PLACA, "%d", ESP.getChipId());
  conecta_wifi();
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setBufferSize(512); // para poder enviar mensajes de hasta X bytes

  conecta_mqtt();

  Serial.printf("Identificador placa: %s\n", ID_PLACA );
  Serial.printf("Termina setup en %lu ms\n\n",millis());
}
//-----------------------------------------------------
#define TAMANHO_MENSAJE 128
unsigned long ultimo_mensaje=0;
//-----------------------------------------------------
//     LOOP
//-----------------------------------------------------
void loop() {
  delay(dht.getMinimumSamplingPeriod());
  if (!mqtt_client.connected()) conecta_mqtt();
  mqtt_client.loop(); // esta llamada para que la librería recupere el control
  unsigned long ahora = millis();

if (ahora - ultimo_mensaje >= 10000) {
    char mensaje[TAMANHO_MENSAJE];
    ultimo_mensaje = ahora;
    //snprintf (mensaje, TAMANHO_MENSAJE, "Mensaje enviado desde %s en %6lu ms", ID_PLACA, ahora);
    //Serial.println(mensaje);    
    struct registro_datos datos;
    
    datos.humedad = dht.getHumidity();
    datos.temperatura = dht.getTemperature();

    Serial.println();
    Serial.printf("Temperatura: %f\n",datos.temperatura);  
    Serial.printf("Humedad: %f\n",datos.humedad);  
  }

}
