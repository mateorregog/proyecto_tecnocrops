/*----Librerias----*/
#include <LoRa.h>
#include <DHT.h>
#include <DHT_U.h>

/*---Directivas---*/
/*Parámetros de comunicación del protocolo LoRa: Spreading Facor(SF), Ancho de banda (BW), Coding Rate (CR)*/
#define banda 915E6  //Banda de transmisión ISM
/*Ancho de banda permitido*/
#define BW1 7.8E3
#define BW2 10.4E3
#define BW3 15.6E3
#define BW4 20.8E3
#define BW5 31.25E3
#define BW6 41.7E3
#define BW7 62.5E3
#define BW8 125E3
#define BW9 250E3
/*Spreading factor permitido*/
#define SF1 7
#define SF2 8
#define SF3 9
#define SF4 10
#define SF5 11
#define SF6 12
/*Coding rate permitido*/
#define CR1 5
#define CR2 8
/*Tiempos de envío*/
#define taire 3000
/*Variable del sensor de humedad y temperatura*/
#define DHTPIN 13 
#define DHTTYPE DHT22
/*-----------------*/

/*Variables temporales para LoRa*/
String sep="V",texto1,texto2, texto3;
/*Variable para generar función del DHT22*/
DHT dht(DHTPIN, DHTTYPE);
float h,t;

/*Inicio del programa y su funcionamiento*/
void setup() {
  Serial.begin(9600);
  Serial.println("Iniciando TTGO...");
  delay(1000);
  inicioLoRa();
  dht.begin();
}

/*Función principal del programa*/
void loop() {
  lecturaSensor();
  enviarPaquete("H"+String(h)+"|T"+String(t)+"|");
  delay(30000);
}

/*---------------------------------------*/
/*Funciones asociadas al dispositivo LoRa*/
/*Función para establecer conexión*/
void inicioLoRa(){
  if(!LoRa.begin(banda)){
    Serial.println("No se ha establecido conexión al dispositivo LoRa...");
    while(1);
  }
  Serial.println("Se ha establecido la conexión LoRa de manera satisfactoria...");
  delay(500);
  Serial.println("...   ...   ...   ...   ...");
  delay(1000);
  configuracionLoRa();
}
/*Función para configurar los parámetros del protocolo LoRa*/
void configuracionLoRa(){
  //Spreading Factor SF: de 7 a 12
  LoRa.setSpreadingFactor(SF1);
  //Ancho de banda BW: de 7.8E3 a 250E3
  LoRa.setSignalBandwidth(BW8);
  //Coding Rate: 5 u 8
  LoRa.setCodingRate4(CR1); 
}
/*Función para envío de paquetes*/
void enviarPaquete(String envio){
  //texto1=envio;
  LoRa.beginPacket();
  LoRa.print(envio);
  Serial.print("El siguiente mensaje se ha enviado correctamente: ");
  Serial.print(envio);
  Serial.println(" ");
  LoRa.endPacket();
}
/*Función para crear texto de envío*/
String textoEnvio(float hum, float tem, int est){
  texto1= hum + sep + tem + sep + est;
  return texto1;
}
/*---------------------------------------*/
/*Función asociada a la lectura del DHT22*/
void lecturaSensor(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("¡La lectura del sensor falló!"));
    return;
  }
  Serial.print(F("Humedad: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.print(F("°C \n"));
  }

/*Función para asignar el valor del tiempo para envío de información*/
int tiempo(int dias,int horas,int minutos){
    int totalD=dias*24*60*60*1000;
    int totalH = horas*60*60*1000;
    int totalM= minutos*60*1000;
    int total=totalD+totalH+totalM;
    return total;
}
/*---------------------------------------*/
