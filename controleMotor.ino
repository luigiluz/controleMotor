/* Eletrônica Digital
 *  Projeto final
 *  Controle de velocidade de motor DC em malha fechada
 *  Alunos: Erick Gabriel e Luigi Luz
 *  Código main
 */

// Obs: ainda falta a função para definir se gira no sentido horário ou no senti anti-horário

#include "controleMotor.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

extern volatile int erro_adc;
extern volatile int valor0;
extern volatile int valor1;
extern volatile long vel;
extern volatile int PID;
extern volatile int setpoint;
extern volatile int erro;
extern volatile int conta;
//extern volatile unsigned int kt;

// ---- configuracao do LCD via I2C ----

LiquidCrystal_I2C lcd(0x3f,2,1,0,4,5,6,7,3, POSITIVE);

// --------------------------------------------------------------------------

// ---- Funcao para imprimir dados no LCD ---
void printLCD(){
  if (conta == 16){
    //Serial.print(erro_adc);
    //Serial.print(' ');
    //Serial.println(valor0);
    //Serial.println(valor1);
    Serial.println(vel);
  lcd.clear();
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("SP = ");
  lcd.print(setpoint);
  lcd.print("rpm");
  lcd.setCursor(0,1);
  lcd.print("MV = ");
  lcd.print(vel);
  lcd.print("rpm");
  
  }
  else if(conta > 16)
  {
    conta = 0;
    }
  }

void setup() {
Serial.begin(9600);
//LiquidCrystal_I2C lcd(0x3f,2,1,0,4,5,6,7,3, POSITIVE);
lcd.begin(16,2);
confT1();
confEI();
cfgADC();
}

void loop() {
ligaPWM();
printLCD();
/*
Serial.print("Velocidade desejada: ");
Serial.print(setpoint);
Serial.print(" Velocidade medida: ");
Serial.print(vel);
Serial.print(" Erro: ");
Serial.print(erro);
Serial.print(" PID: ");
Serial.println(PID);
*/
}
