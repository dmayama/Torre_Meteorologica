/*Proyecto torre meteorologica*/
/*Se incluyen librerías instaladas*/
#include "UbidotsEsp32Mqtt.h"
#include "DHT.h"
#include "max6675.h"
/*Definimos que el Pin de Datos del sensor DHT es el 23 del ESP32*/
#define DHTPIN 23           
/*Definimos que el tipo de sensor que estamos utilizando es un DHT11*/
#define DHTTYPE DHT11     
/*Se actualizan algunos parámetros del DHT11 con los puntos definidos anteriormente*/ 
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>
/*Se utiliza la libreria para obtener los datos del sensor DHT11*/
DHT dht(DHTPIN, DHTTYPE);
/*SE definen los puertos utilizados para el sensor MAX6675*/
int ktcSO = 27;
int ktcCS = 14;
int ktcCLK = 12;
/*Se utiliza la libreria instalada para obtener los datos del sensor MAX6675*/
MAX6675 ktc(ktcCLK, ktcCS, ktcSO);
/*Se definen variables para sincronizar en linea con la plataforma Ubidots*/
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
/*Definimos primero el Token que nos brinda la plataforma Ubidots para hacer la conexión*/
const char *UBIDOTS_TOKEN = "BBFF-0vPCwFEJop19aaloIxvQzFR37pK2GU";
/*Definimos SSID y PASSWORD de nuestra red WiFi*/
const char *WIFI_SSID = "Donchi";      
const char *WIFI_PASS = "Xllnn0690X2";      
/*Definimos el nombre de nuestro dispositivo, el cual aparecerá en la plataforma Ubidots*/
const char *DEVICE_LABEL = "Variables";
/*Definimos las variables que se medirán y que serán publicadas en la plataforma Ubidots*/
const char *VARIABLE_LABEL_1 = "temperatura"; 
const char *VARIABLE_LABEL_2 = "humedad"; 
const char *VARIABLE_LABEL_3= "termocupla";
const char *VARIABLE_LABEL_4= "lux";
const char *VARIABLE_LABEL_5= "lluvia";
/*Definimos la frecuencia de publicación de 5 segundos*/
const int PUBLISH_FREQUENCY = 5000; 
/*Definimos unas variables extra, que incluye la librería*/
unsigned long timer;
/*Se define pin analogo para la medicion obtenida por el sensor*/
uint8_t analogPin = 34; 
/*Definimos que las variables de la librería UBIDOTS trabajarán con el Token*/
Ubidots ubidots(UBIDOTS_TOKEN);
/*La librería nos da una función de Subscripción para mensajes de regreso desde la plataforma Ubidots hacia nuestro circuito*/
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


/*Definimos la función Setup() para iniciar nuestro circuito*/
void setup()
{
  /*Iniciamos el terminal Serial a 115200*/
  Serial.begin(115200);     
  /*Imprimimos el siguiente texto para asegurar el encendido del circuito*/
  Serial.println("Medición inciada");
  /*Iniciamos nuestro sensor DHT11*/
  dht.begin();
  
  tsl.enableAutoRange(true); 
  
  tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);
  /*Incluimos un retardo de 1 segundo*/
  delay(1000);
  /*Ahora, se incluye las funciones de conexión de la Plataforma, como la conexión a internet con las credenciales de WiFi*/
  // ubidots.setDebug(true);  //Descomentar esto para que los mensajes de depuración estén disponibles
  ubidots.connectToWifi(WIFI_SSID, WIFI_PASS);
  /*Se inicia la función de subscripción para los mensajes de retorno, que se definió líneas arriba*/
  ubidots.setCallback(callback);
  /*Se incluye las funciones de Setup y Reconnect, predefinidas de la librería*/
  ubidots.setup();
  ubidots.reconnect();
  timer = millis();
}

/*Iniciamos el bucle de la función Loop()*/
void loop()
{
  /*Definimos que, si no se conecta a la plataforma Ubidots, se pasa la función Reconnect() para volver a establecer la conexión*/
  if (!ubidots.connected())
  {
    ubidots.reconnect();
  }
  /*En esta condicional, iniciamos la medición de Temperatura y Humedad del sensor, y la información será enviada a la Plataforma*/
  if (abs(millis() - timer) > PUBLISH_FREQUENCY) 
  {
    sensors_event_t event;
    tsl.getEvent(&event);
    /*Hacemos la medición de Temperatura,Humedad,lluvia,lumines y lo definimos en variables de tipo Float */
    float lluvia=digitalRead(13);
    float l= event.light;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float tempc=ktc.readCelsius();
    /*Definimos que el valor de los diferentes sensores que será enviado a la plataforma Ubidots*/
    ubidots.add(VARIABLE_LABEL_1, t);
    ubidots.add(VARIABLE_LABEL_2, h);
    ubidots.add(VARIABLE_LABEL_3, tempc);
    ubidots.add(VARIABLE_LABEL_4, l);
    ubidots.add(VARIABLE_LABEL_5, lluvia);
    /*Hacemos la publicación de los datos en el dispositivo definido*/
    ubidots.publish(DEVICE_LABEL);
    /*Para mostrar los datos, los imprimimos en el terminal Serial*/
    Serial.println("Enviando los datos a Ubidots: ");
    Serial.println("Temperatura: " + String(t));
    Serial.println("Humedad: " + String(h));
    Serial.println("lluvia: " + String(lluvia));
    Serial.println("Temperatura: " + String(tempc));
    Serial.println("Lux: " + String(l));
    Serial.println("-----------------------------------------");
    timer = millis();
  }
  /*Agregamos un delay de 10 segundos para el envío, y que luego de este tiempo, se proceda a reiniciar el bucle*/
  delay(10000); 
  ubidots.loop();
}
