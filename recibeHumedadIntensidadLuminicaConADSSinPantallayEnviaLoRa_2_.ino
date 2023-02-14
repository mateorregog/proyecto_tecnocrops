#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>
//#include <avr/dtostrf.h>  //Se usa para convertir los float en cadenas
 
/*Dirección I2C del ADS115, puede ser 0X48, 0X49, 0X4A, 0X4B*/
#define ADS1 0x48  //Ground
#define ADS2 0x49 //VDD 
#define ADS3 0x4A  //SDA
#define ADS4 0x4B //SCL

/*Registros de lectura y escritura del ADS1115*/
#define REGL 0x00  //Registro de conversión
#define REGC 0x01  //Registro de configuración del ADS1115
#define REGLT 0X02 //Registro bajo de disparo de alerta
#define REGHT 0X03 //Registro alto de disparo de alerta

//Banda de transmision LoRa
#define BAND 915E6 
int contador; //Cuenta las emisiones lora

//Hacer líneas de comentarios múltiples con valores del datasheet en cada función.
//Fragmentar la variable a en 2 para 8 bit y 8 bits.

const char* ssid = "Moli";
const char* password = "cocoYMOLI1542";

// Domain Name with full URL Path for HTTP POST Request
const char* serverName = "http://api.thingspeak.com/update";

// Service API Key
String apiKey = "FE0H73X026LA68F3";

//Desclaración de los vectores de configuración
uint16_t a=0b00000000000000000;
uint8_t b=0b00000000;
uint8_t c=0b00000000;
uint16_t lec;
float res;

//Asignación de los bits de configuración

//OS: Operational status/single-shot conversion start. Números 0 o 1.
void bit15(uint8_t x){
  uint8_t temp=x<<7; //Corrimiento de bits
  b=b|temp;
}

//Input multiplexer configuration. En decimal números de 0 a 7. (Consultar datasheet para ver la configuración detallada) 4 = A0; 5 = A1 ; 6 = A2 ; 7 = A3
void bit14a12(uint8_t x){
  uint8_t temp=x<<4;
  b=b|temp;
}

// Programmable gain amplifier configuration. Números de 0 a 7 donde:
// 0 : FS = ±6.144V(1), 1 : FS = ±4.096V(1), 2 : FS = ±2.048V (default), 3 : FS = ±1.024V,  4 : FS = ±0.512V, 5 : FS = ±0.256V, 6 : FS = ±0.256V, 7 : FS = ±0.256V
void bit11a9(uint8_t x){
  uint8_t temp=x<<1;
  b=b|temp;
}

// Device operating mode. 0 : Continuo , 1:Power-down single-shot (default)
void bit8(uint8_t x){
  uint8_t temp=x;
  b=b|temp;
}

// Data rate. Números de 0 a 7 donde:
//0 : 8SPS , 1 : 16SPS , 2 : 32SPS , 3 : 64SPS, 4 : 128SPS (default), 5 : 250SPS, 6 : 475SPS, 7 : 860SPS 
void bit7a5(uint8_t x){
  uint8_t temp=x<<5;
  c=c|temp;
}

//Comparator mode. Números 0 o 1.
// 0 : Comparador tradicional con hysteresis (default), 1 : Comparador de ventana (Window comparator)
void bit4(uint8_t x){
  uint8_t temp=x<<4;
  c=c|temp;
}

//Comparator polarity. Números 0 o 1.
// 0 : Active low , 1 : Active high
void bit3(uint8_t x){
  uint8_t temp=x<<3;
  c=c|temp;
}

//Latching comparator. Números 0 o 1.
//0 : Non latching comparator (Default), 1 : Lartching comparator
void bit2(uint8_t x){
  uint8_t temp=x<<2;
  c=c|temp;
}

//Comparator queue and disable
void bit1a0(uint8_t x){
  uint8_t temp=x;
  c=c|temp;
}

//Crea la cadena de 2 bytes
void formar2bytes(){
    a=b<<8;
    a=a|c;   
}

void configuracionInicialAds(int x){
    bit15(0);
    bit14a12(x);
    bit11a9(1);
    bit8(0);
    bit7a5(1);
    bit4(0);
    bit3(0);
    bit2(0);    
    bit1a0(0);
    formar2bytes();
 }

void configurarADS1115(uint8_t dir,uint8_t reg,uint8_t VH,uint8_t VL){
  Wire.beginTransmission(dir);
  Wire.write(reg);
  Wire.write(VH);
  Wire.write(VL);
  Wire.endTransmission();
}

void verificarRegistro(uint8_t dir,uint8_t reg, int x){    // La función recibe como parámetro la dirección, el tipo de acceso y un último parámetro que verifica si se quiere leer la configuración de registro o leer datos capturados.
  Wire.beginTransmission(dir);
  Wire.write(reg);
  Wire.endTransmission();
  Wire.requestFrom(dir,2);
  lec=Wire.read()<<8|Wire.read();
  if (x==1){
      Serial.print("Lectura registro configuración : ");
      Serial.println(lec,BIN);
    }
  else if (x==2){
      Serial.print("La lectura del sensor es: ");
      res=(lec*4.096)/32768;
      Serial.print(res);
      Serial.println(" Voltios");
    }
}
//Sensor de humedad a0=4     Sensor intensidad luminica a1=5
void sensorLuz(){
  verificarRegistro(ADS2,REGL,2); //Imprime el valor de la lectura.
  verificarRegistro(ADS2,REGC,1);//Imprime el registro de configuración para ver qué entrada está leyendo.
  }

void calculoYL100(){
  verificarRegistro(ADS2,REGL,2); //Imprime el valor de la lectura
  verificarRegistro(ADS2,REGC,1); 
}

void setup() {
  Serial.begin(9600);
  Wire.begin();
  configuracionInicialAds(4);
  configurarADS1115(ADS2,REGC,b,c);
  Serial.println("LoRa Sender");
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initial OK!");
}

void loop() {
  Serial.print("Sensor humedad: ");
  calculoYL100();
  configuracionInicialAds(5); //Escribe el registro para leer luz
  configurarADS1115(ADS2,REGC,b,c); // Configura el ads con el nuevo registro
  a=0;b=0;
  float dato1=res;
  delay(5000);
  Serial.print("Sensor luz: ");
  sensorLuz();
  Serial.print("Enviando paquete: ");
  Serial.println(contador);
  contador++;
  LoRa.beginPacket();
  LoRa.print("Sensor de humedad: ");
  LoRa.print("|");
  LoRa.print(dato1);
  LoRa.print("|");
  configuracionInicialAds(4);//Escribe el registro para leer humedad
  configurarADS1115(ADS2,REGC,b,c);   //Configura ads con nuevo registro
  a=0;b=0;
  delay(15000);
  float dato2=res;
  LoRa.print("Sensor de luz: ");
  LoRa.print("|");
  LoRa.print(dato2); 
  LoRa.endPacket();
}
