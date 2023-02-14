/*Librería necesarias*/
#include <LoRa.h>
#include <WiFi.h>
#include <HTTPClient.h>

/*Directivas del programa*/

#define BANDA    915E6
#define BW1 7.8E3
#define BW2 10.4E3
#define BW3 15.6E3
#define BW4 20.8E3
#define BW5 31.25E3 //Funcionar desde acá
#define BW6 41.7E3
#define BW7 62.5E3
#define BW8 125E3
#define BW9 250E3

/*Variables del programa*/
float datoHF, datoLF, y, z;
String valorFinal,datoH="",datoL="";
String valorStringHumedad;
String valorStringLuz;
bool lectura=false;

//Credenciales para la red wifi
const char* ssid = "Jonathan";
const char* password = "jhnp9010";

// Dominio al que se conecta para enviar los mensajes POST vía HTTP
const char* serverName = "http://api.thingspeak.com/update";

// Clave API del servicio
String apiKey = "FE0H73X026LA68F3";

void setup(){ 
  Serial.begin(9600);
  iniciarLoRa();
  WiFi.begin(ssid, password);
  Serial.println("Conectando");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Conectado a la red WiFi con dirección IP: ");
  Serial.println(WiFi.localIP());
  delay(1000);
}

void loop() {
  String valor=lecturaLoRa();
  if(lectura==true){
    Serial.println(valor);
    lectura=false;
    envioDatosPost();
    datoH="";
    datoL="";
    y=0;
    z=0;
  }
}

/*------------------------------------*/

/*Función para configurar e iniciar protocolo LoRa*/

void iniciarLoRa(){
  
  Serial.println("Conectando Receptor LoRa...");
  if (!LoRa.begin(BANDA)) {
    //Serial.println("Conexión LoRa Fallida. Intente de nuevo...");
  }
  //Serial.println("Listo para recibir datos.");
  delay(2000); 
  //Spreading Factor SF: de 7 a 12
  LoRa.setSpreadingFactor(7);
  //Ancho de banda BW:  7.8E3 A 250E3
  LoRa.setSignalBandwidth(BW8);
  //Coding rate: puede ser 5 u 8
  LoRa.setCodingRate4(5);
}

/*-----------------------------------------------*/

/*Función para leer mensajes del Protocolo LoRa*/

String lecturaLoRa(){
  //Se verifica que el paquete tenga más de 1 carácter enviado
  String texto;
  char temporal;
  int tamanoPaquete;
  tamanoPaquete = LoRa.parsePacket();
  if (tamanoPaquete) {
    //El paquete ha sido recibido
    //Serial.println("Paquete recibido.");
    //Se hace la lectura del paquete recibido
    while (LoRa.available()) {
      //Serial.print((char)LoRa.read());
      temporal=(char)LoRa.read();
      //Serial.println(temporal);
      
       
      if(temporal=='H'){
        bool bandera=true;
        while(bandera){
          temporal=(char)LoRa.read();
          if(temporal == '|'){
            bandera=false;
            datoHF = datoH.toFloat();
            float m= (0-100)/(3.3-1.64);
            float b= -m*3.3;
            y = m*datoHF+b;
            //Serial.println(y);
            valorStringHumedad=String(y,2); 
            
          }
          else{
            datoH =datoH+temporal;
            
          
          }
        }
      } 

      if(temporal=='L'){
        bool bandera=true;
        while(bandera){
          temporal=(char)LoRa.read();
          if(temporal == '|'){
            bandera=false;
            datoLF = datoL.toFloat();
            float z = datoLF * 30;
            valorStringLuz = String(z,2);
            
            
          }
          else{
            datoL =datoL+temporal;
          }
        }
      } 
      
    
    texto="Humedad: "+datoH+" "+valorStringHumedad+"\n Luz: "+datoL+" "+valorStringLuz;
    lectura=true;
  }
  return texto;
}
}


/*-----------------------------------------------*/

/*Función para realizar el envío a la página de ThingSpeak*/
void envioDatosPost(){
  if(WiFi.status()== WL_CONNECTED){
      WiFiClient client;
      HTTPClient http; 
      //Su nombre de dominio con la ruta URL o la dirección IP con la ruta
      http.begin(client, serverName);    
      //Especificar cabecera de tipo de contenido
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");    
      //Datos a enviar con HTTP POST
      String httpRequestData = "api_key=" + apiKey + "&field1=" + valorStringLuz  + "&field2=" + valorStringHumedad;          
      //Enviar solicitud HTTP POST
      int httpResponseCode = http.POST(httpRequestData);
      Serial.print("Código de respuesta HTTP: ");
      Serial.println(httpResponseCode);     
      //Libera los recursos
      http.end();
    }
  else {
      Serial.println("WiFi se encuentra desconectado.");
    }
}
