//PLACA DE MIGUE--> ESP_3097139
//-----------------------------------------------------
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "DHTesp.h"

#include <Wire.h>
#include <Adafruit_INA219.h>

DHTesp dht;
ADC_MODE(ADC_VCC);

Adafruit_INA219 ina219;

const int pinTCRT5000 = 10;

//-----------------------------------------------------

struct registro_datos {
        float caida_tension;
        float corriente;
        float temperatura;
        float humedad;
        int movimiento;
        };

WiFiClient wClient;
PubSubClient mqtt_client(wClient);

//#define Lugar1   //Casa
#define Lugar2    //Teleco
//#define Lugar3    //Movil

#ifdef Lugar1
#define ssid "DIGIFIBRA-f2CX"
#define password "ekhXP6XD4S"
#endif

#ifdef Lugar2 
#define ssid "huerticawifi"
#define password "4cc3sshu3rt1c4"
#endif

#ifdef Lugar3 
#define ssid "iPhone"
#define password "aaaaaaaa"
#endif

//-----------------------------------------------------
const char* mqtt_server = "172.16.135.208"; // "iot.ac.uma.es";
//-----------------------------------------------------
// Introducimos los valores del servidor MQTT
//const char* mqtt_user = "infind";//"II3";
//const char* mqtt_pass = "zancudo";//"79qUYsdM";
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
  char prueba_mqtt[128]; 
  // A continuación, creamos una cadena de caracteres de 128 de longitud donde guardaremos 
  // el mensaje de ultima voluntad indicando que nos hemos desconectado repentinamente
  snprintf(cadenaConectado, 128,"{\"online\": false}");
  snprintf(cadenaDesconectado, 128,"{\"online\": true}");
  snprintf(prueba_mqtt, 128,"Conexion establecida");
  // Mienrtas que no establezcamos la conexion no saldremos del bucle
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(ID_PLACA/*,mqtt_user,mqtt_pass,"II3/ESP3097139/conexion",2,true,cadenaDesconectado*/)) {
      Serial.printf(" conectado a broker: %s\n",mqtt_server);
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      //mqtt_client.subscribe();
      mqtt_client.publish("TFG/DHT22_conexion_establecida",prueba_mqtt);
    } else {
      Serial.printf("failed, rc=%d  try again in 5s\n", mqtt_client.state());
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//-----------------------------------------------------
//Con la siguiente funcion conseguiremos crear un Json para poder enviarlo por MQTT 
String serializa_JSON (struct registro_datos datos)
    {
      StaticJsonDocument<300> jsonRoot;
      String jsonString;

      JsonObject DHT22=jsonRoot.createNestedObject("DHT22");
      DHT22["temp"] = datos.temperatura;
      DHT22["hum"] = datos.humedad;
      
      JsonObject INA219=jsonRoot.createNestedObject("INA219");
      INA219["Corriente"] = datos.corriente;
      INA219["Tension"] = datos.caida_tension;

      JsonObject TCRT5000=jsonRoot.createNestedObject("TCRT5000");
      TCRT5000["Puerta"] = datos.movimiento;
      
      serializeJson(jsonRoot,jsonString);
      return jsonString;
    }
//-----------------------------------------------------
//     SETUP
//-----------------------------------------------------
void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Empieza setup...");
  pinMode(pinTCRT5000,INPUT);
  dht.setup(0, DHTesp::DHT22); // Connect DHT22 sensor to GPIO 0
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

  uint32_t currentFrequency;

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }
  

if (ahora - ultimo_mensaje >= 10000) {
    char mensaje[TAMANHO_MENSAJE];
    ultimo_mensaje = ahora;
    //snprintf (mensaje, TAMANHO_MENSAJE, "Mensaje enviado desde %s en %6lu ms", ID_PLACA, ahora);
    //Serial.println(mensaje); 

    struct registro_datos datos;

    
    datos.movimiento = digitalRead(pinTCRT5000);


    datos.humedad = dht.getHumidity();
    datos.temperatura = dht.getTemperature();
    
    datos.caida_tension= ina219.getBusVoltage_V();
    datos.corriente=ina219.getCurrent_mA();
    
    Serial.printf("Temperatura: %f\n",datos.temperatura);  
    Serial.printf("Humedad: %f\n",datos.humedad); 
    
    Serial.print("Tension VCC:   "); Serial.print(datos.caida_tension); Serial.println(" V");
    Serial.print("Corriente:       "); Serial.print(datos.corriente); Serial.println(" mA");


    Serial.print("Puerta ('0' cerrada '1' abierta)= ");
    Serial.print(datos.movimiento);
    
    Serial.println("\n");
    
    mqtt_client.publish("TFG/DHT22_INA219",serializa_JSON(datos).c_str());
    
  }

}
