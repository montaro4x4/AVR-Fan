#include "SevSeg.h"
/*** AUTOR: Montaro Hazlo Tu Mismo ***/
/*** Fecha: 2021-05-29 ***/
/*** https://www.youtube.com/channel/UCuN8Y9hTLnW-v3Yt7URU6ew/featured ***/
SevSeg sevseg; 
/********* Asignacion de PUERTOS *********/
int termistorPinA0 = 0;
int fanBajaPin = 15;
int fanAltaPin = 16;
int botonPin = 12;
int receptorAVPin = 17;
/********************************/
int switchEstadoAnterior = 0;     // ***** Auxiliar para baton *****
bool displayOn = false;           // ***** Modo del display ****
/*********** Variables para controlar ventilador ************/
uint32_t tiempoTemp=millis();
uint32_t ms_Temp=1000;              //(milisegundos) Frecuencia de revision de temperatura 1 Seg = 1000 
uint32_t ventiladorMillis=millis(); //(milisegundos) Auxiliar para calcular el tiempo de ventilador encendido 
uint32_t tiempoVentiladorAlta=10;    //(Minutos) Tiempo que dura en ventilador en alta al alcanzar la temperatura de alta tempVentiladorAlta
int tempVentiladorAlta=45;          //(Grados Centigrados) Temperatura a la que encendera en alta el ventilador 
bool ventiladorAltaON;              //Mantiene el estado del ventilador en alta para darle tiempo 
/****************************************************************/

void setup()
{
  /****** Habilitar Serial Temporal *****/
  //Serial.begin(9600);
  //Serial.println("Inicializando");  
  /****** Inicializar 7 Secmentos ******/
  byte numeroDigitos = 2;
  byte digitosPins[] = {11, 10};
  byte segmentoPins[] = {3, 4, 5, 6, 7, 8, 9, 2};
  bool resistenciasEnSegmentos = true; 
  bool updateWithDelaysIn = true;
  byte tipoDisplay = COMMON_CATHODE; 
  sevseg.begin(tipoDisplay, numeroDigitos, digitosPins, segmentoPins, resistenciasEnSegmentos);  
  /***** Inicializar Puertos ******/
  pinMode(botonPin,INPUT );  
  pinMode(receptorAVPin,INPUT);  
  pinMode(fanBajaPin,OUTPUT);
  pinMode(fanAltaPin,OUTPUT);
  /********* Cambio de Minutos a Milisegundos  **********/
  tiempoVentiladorAlta=tiempoVentiladorAlta*60*1000;
  /********* Inicializa los estados *************/ 
  sevseg.setChars(".");
  digitalWrite(fanBajaPin,LOW);
  digitalWrite(fanAltaPin,LOW);
  ventiladorAltaON=false;
}

void loop()
{ 
  ReceptorAV();
  Boton();
  RevisaTemperatura();
  ApagadoVentiladorAlta();
  sevseg.refreshDisplay();
}

void ReceptorAV()
{
  if(digitalRead(receptorAVPin))
  {
    digitalWrite(fanBajaPin,HIGH);
  }
  else
  {
    digitalWrite(fanBajaPin,LOW);
  }  
}

void Boton()
{
  if(digitalRead(botonPin)==HIGH & switchEstadoAnterior==0)
  { 
    switchEstadoAnterior=1;
    if(displayOn)
    {
      displayOn=false;     
      sevseg.blank();          
      sevseg.refreshDisplay();
      sevseg.setChars(".");
      sevseg.refreshDisplay();      
    }
    else
    {
      displayOn=true;                
      sevseg.blank();
      sevseg.refreshDisplay();
      sevseg.refreshDisplay();
    }                           
    delay(200);               
  }
  if(digitalRead(botonPin)==LOW & switchEstadoAnterior == 1)
    switchEstadoAnterior=0;
}


void RevisaTemperatura()
{
  uint32_t capturaMillis = millis();
  if ((capturaMillis-tiempoTemp) < 0)
     tiempoTemp=0;
  if ((capturaMillis-tiempoTemp) >= ms_Temp) 
  {
    if (capturaMillis >= 4294967295-ms_Temp)
      tiempoTemp = 0;
    else
      tiempoTemp = capturaMillis;
    int TempRounded = round(Temperatura());
    if(TempRounded >= tempVentiladorAlta && digitalRead(receptorAVPin) )
    {
      digitalWrite(fanBajaPin,LOW);
      digitalWrite(fanAltaPin,HIGH);
      if(!ventiladorAltaON)
      {
        if (capturaMillis >= 4294967295-ms_Temp)
          ventiladorMillis = 0;
        else
          ventiladorMillis = capturaMillis;        
        Serial.print("Prendido");
      }
      ventiladorAltaON=true; 
    }
    if(displayOn) 
    {
      if(TempRounded > 99)
      {
        sevseg.setChars("HI");
      }
      else
      {
        sevseg.setNumber(TempRounded);  
      }
    }    
    Serial.print("Tempertura: "); Serial.print(Temperatura(),10); Serial.println(" °C");   
    //Serial.println(tiempoTemp);
    Serial.print("Receptor AV: "); Serial.println(digitalRead(receptorAVPin));
    Serial.print("Millis Actual: "); Serial.println(capturaMillis);     
  }
}

void ApagadoVentiladorAlta()
{
  uint32_t capturaMillis = millis();
  if (ventiladorAltaON)
  {
    if ((capturaMillis-ventiladorMillis) < 0)
       ventiladorMillis=0;
    if ((capturaMillis-ventiladorMillis) >= tiempoVentiladorAlta) 
    {
      Serial.print("Apagado");
      if (capturaMillis >= 4294967295-tiempoVentiladorAlta)
        ventiladorMillis = 0;
      else
        ventiladorMillis = capturaMillis;
      digitalWrite(fanAltaPin,LOW);
      ventiladorAltaON=false;         
    }
  }
}




float Temperatura()
{ 
  int lecturaTermistor = analogRead(termistorPinA0); //Lectura Puerto Analogico 
  float Tref = 25+273.15;  /*** Temperatura Nominal (°K)  ***/
  double Rref = 100000;  /*** Resistencia Nominal ***/
  double B = 3950;
  float Rter = (lecturaTermistor*Rref)/(1023-lecturaTermistor);
  /****** FORMULA steinhart:  1/T = 1/Tref + (1/B x log(R/Rn) ******/  
  float TemperaturaSteinhart = 1/((1/(Tref))+((1/B)*(log((Rter/Rref)))));
  float TemperturaC = TemperaturaSteinhart-273.15;
  return TemperturaC;
}








