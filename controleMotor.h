/* Eletrônica Digital
 *  Projeto final
 *  Controle de velocidade de motor DC em malha fechada
 *  Alunos: Erick Gabriel e Luigi Luz
 *  Professor: Roberto Kenji Hiramatsu
 *  
 *  Cabeçalho da biblioteca controleMotor
 */

 #ifndef _CONTROLEMOTOR_H
 #define _CONTROLEMOTOR_H
 #include <Arduino.h>
 #include <Wire.h>
 #include <LiquidCrystal_I2C.h>

int pwm2Vel (int pwm);
int vel2Pwm (int vel);
void confT1();
ISR(TIMER1_OVF_vect);
void ligaPWM();
void desligaPWM();
void confEI();
ISR(INT0_vect);
ISR(ADC_vect);
void cfgADC();
void trADC();
int adc2Pwm (int val0 ,int val1);
void cPID();
void pontoMorto();
void freio();
void antiHorario();
void horario();
#endif
