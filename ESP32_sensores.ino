#include <WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

const char* ssid = "MOVISTAR_27D0"; //Nombre del router
const char* password = "hQwNroR7pKxMCUxWpULS"; //Contraseña
const char* mqtt_server = "192.168.1.42"; //Dirección IP de la Raspberry PI
int VoltagePin = 36; //Se asigna el GPIO 36 a la lectura de voltaje. (ADC0_CH0). Pin 4 del ESP32
int VoltageLecture = 0;
int VoltageValue = 0;
int CurrentPin = 39; //Se asigna el GPIO 39 a la lectura de corriente. (ADC0_CH3). Pin 5 del ESP32
int CurrentLecture = 0;
int CurrentValue = 0;
OneWire ourWire(34); //Se asigna el GPIO 34 a la lectura de temperaturas. (Bus One-Wire). Pin 6 del ESP32
DallasTemperature sensors(&ourWire);
DeviceAddress address1 = {0x28,0x0E,0x2D,0x73,0x0A,0x00,0x00,0x53}; //ID del sensor 1
DeviceAddress address2 = {0x28,0x62,0x3E,0x72,0x0A,0x00,0x00,0x50}; //ID del sensor 2
DeviceAddress address3 = {0x28,0x1E,0xBF,0x73,0x0A,0x00,0x00,0x3C}; //ID del sensor 3
DeviceAddress address4 = {0x28,0x6E,0xD0,0x72,0x0A,0x00,0x00,0x55}; //ID del sensor 4
DeviceAddress address5 = {0x28,0x1B,0x17,0x14,0x09,0x00,0x00,0x28}; //ID del sensor 5
DeviceAddress address6 = {0x28,0xC7,0x1A,0x74,0x0A,0x00,0x00,0x09}; //ID del sensor 6
float temp1;
float temp2;
float temp3;
float temp4;
float temp5;
float temp6;
int tempmax;


uint16_t* SPI_DMA_IN_LINK_REG=0x108;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() { //Setup del modulo Wifi

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(4, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP32)
  } else {
    digitalWrite(4, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int maxtemp(float temp1, float temp2, float temp3, float temp4, float temp5, float temp6) { // Cálculo de la temperatura maxima
  int tempmax = 0;
  float temps[6] = {temp1,temp2,temp3,temp4,temp5,temp6};
  for (i=1;i<=6;i++)
    {
      if (temps[i]>tempmax)
      {
        tempmax = temps[i];
      }
    }
  return tempmax;
}

void setup() {
  pinMode(4, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  *SPI_DMA_IN_LINK_REG=0x20140440;
  sensors.begin(); //Inicio de los DS18B20
}

void loop() {
    if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) { // Lectura de sensores y publicación MQTT cada 2 segundos
    lastMsg = now;
    
    VoltageLecture = analogRead(VoltagePin); //Lectura de tensión
    CurrentLecture = analogRead(CurrentPin); //Lectura de corriente
    VoltageValue = map(VoltageLecture,0,4095,0,60); //Conversión de bits a voltios
    CurrentValue = map(CurrentLecture,0,4095,-28.7,28.7); //Conversión de bits a Amperios
    sensors.requestTemperatures(); //Comando para obtener temperaturas
    temp1 = sensors.getTempC(adress1); //Obtención de la temperatura en ºC del sensor 1
    temp2 = sensors.getTempC(adress2); //Obtención de la temperatura en ºC del sensor 2
    temp3 = sensors.getTempC(adress3); //Obtención de la temperatura en ºC del sensor 3
    temp4 = sensors.getTempC(adress4); //Obtención de la temperatura en ºC del sensor 4
    temp5 = sensors.getTempC(adress5); //Obtención de la temperatura en ºC del sensor 5
    temp6 = sensors.getTempC(adress6); //Obtención de la temperatura en ºC del sensor 6
    tempmax = maxtemp(temp1,temp2,temp3,temp4,temp5,temp6); //Cálculo de la temperatura máxima

    //Publicación de mensajes MQTT
    snprintf (msg, 50, "%d", VoltageValue);
    Serial.print("Publish message: ");
    Serial.println(VoltageValue);
    client.publish("panel1/voltage", msg);
    snprintf (msg, 50, "%d", CurrentValue);
    Serial.print("Publish message: ");
    Serial.println(CurrentValue);
    client.publish("panel1/current", msg);
    snprintf (msg, 50, "%d", tempmax);
    Serial.print("Publish message: ");
    Serial.println(tempmax);
    client.publish("panel1/temperature", msg);
  }

}
