/*Creación de tareas*/
TaskHandle_t Tarea1; //Este se identifica como el manejador de la tarea u objeto PH
TaskHandle_t Tarea2; //Este se identifica como el manejador de la tarea u objeto TDS
TaskHandle_t Tarea3; //Este se identifica como el manejador de la tarea u objeto Temperatura Agua

#include <Wire.h>
#include <LoRa.h>
#include <OneWire.h> //Librerías  sensor temperatura agua
#include <DallasTemperature.h> //Sensor temperatura agua
 
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

//Desclaración de los vectores de configuración
uint16_t a=0b00000000000000000;
uint8_t b=0b00000000;
uint8_t c=0b00000000;
uint16_t lec;
double res;

// GPIO where the DS18B20 is connected to
const int oneWireBus = 13;  

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(oneWireBus);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

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
    bit11a9(0);
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
      res=(lec*6.144)/(32768);
      Serial.print(res,6);
      Serial.println(" Voltios");
    }
}

void setup() {
  Serial.begin(9600);
  // Start the DS18B20 sensor
  sensors.begin();
  
  xTaskCreatePinnedToCore(
    lectura_pH,//Función que implementa la tarea
    "Tarea1", //Nombre de la tarea
    10000, //Tamaño de la pila en palabras
    NULL, //Parámetros de entrada de la tarea
    10, //Prioridad de la tarea (0 es la más baja)
    &Tarea1, //Objeto creado para la tarea
    0); //Núcleo en el cual correrá la tarea

   xTaskCreatePinnedToCore(
    lectura_TDS,//Función que implementa la tarea
    "Tarea2", //Nombre de la tarea
    10000, //Tamaño de la pila en palabras
    NULL, //Parámetros de entrada de la tarea
    6, //Prioridad de la tarea (0 es la más baja)
    &Tarea2, //Objeto creado para la tarea
    0); //Núcleo en el cual correrá la tarea

   xTaskCreatePinnedToCore(
    lectura_Temperatura,//Función que implementa la tarea
    "Tarea3", //Nombre de la tarea
    10000, //Tamaño de la pila en palabras
    NULL, //Parámetros de entrada de la tarea
    3, //Prioridad de la tarea (0 es la más baja)
    &Tarea3, //Objeto creado para la tarea
    0); //Núcleo en el cual correrá la tarea
    
    
    Serial.begin(9600);
    Wire.begin();
    
}

void loop() {
  // put your main code here, to run repeatedly:
    Serial.println("Núcleo: "+ String(xPortGetCoreID()));
    delay(4000);   
}

void lectura_pH(void * parameter){
  for(;;){
    Serial.println("Sensor PH: ");
    verificarRegistro(ADS2,REGL,2); //Imprime el valor de la lectura.
    verificarRegistro(ADS2,REGC,1);//Imprime el registro de configuración para ver qué entrada está leyendo.
    configuracionInicialAds(5); //Escribe el registro para leer tds en el A1
    configurarADS1115(ADS2,REGC,b,c); // Configura el ads con el nuevo registro
    a=0;b=0;res=0;
    delay(5000);
    
  }
}



void lectura_TDS(void * parameter){
  for(;;){   
    Serial.println("Sensor TDS: ");
    verificarRegistro(ADS2,REGL,2); //Imprime el valor de la lectura.
    conversion_tds();
    verificarRegistro(ADS2,REGC,1);//Imprime el registro de configuración para ver qué entrada está leyendo.
    configuracionInicialAds(4); //Conectar sensor ph en A0
    configurarADS1115(ADS2,REGC,b,c);
    a=0;b=0;res=0;
    delay(5000);
  }
}


void lectura_Temperatura(void * parameter){
  for(;;){   
    Serial.println("Sensor Temperatura: ");
    sensors.requestTemperatures(); 
    float temperatureC = sensors.getTempCByIndex(0);
    Serial.print(temperatureC);
    Serial.println("ºC");
    delay(5000);
  }
}


double conversion_tds(){
  double x=(res*1000)/3.3;
  Serial.println("El valor de PPM es: "+String(x));
}
