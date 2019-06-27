/* Eletrônica Digital
 *  Projeto final
 *  Controle de velocidade de motor DC em malha fechada
 *  Alunos: Erick Gabriel e Luigi Luz
 *  Funções que serão implementadas com a biblioteca
 */

 #include "controleMotor.h"
 #include <Wire.h>
 #include <LiquidCrystal_I2C.h>
 
// ---- Função para converter valores de pwm em valores de velocidade ----

int pwm2Vel (int pwm){
  int vel;
  if (pwm < 49){
    vel = 0;
    }
  else if((pwm >= 49) && (pwm <= 255)){
    vel = 9*pwm - 211;
    }
  else{
    vel = 2084;}
  return vel;}

// --------------------------------------------------------------------------

// ---- Função para converter valores de velocidade em valores de pwm ----

int vel2Pwm (int vel){
  int pwm;
    if (vel < 230){
      pwm = 0;}
    else if((vel >= 230) && (vel <= 2084)){
      pwm = (vel+211)/9;
      }
    else{
      pwm = 255;}
    return pwm;}

// --------------------------------------------------------------------------

// ---- Função para configurações do timer 1 ----

// Configurações para o timer 1
void confT1(){
  // Resetando as configurações dos registradores de controle
  TCCR1A = 0;
  TCCR1B = 0;
  // Configuração de WGM1 para operar no modo:
  // PWM, Phase Correct, 8 bits
  // A configuração de WGM1 necessária será: WGM1(3:0) = 0001
  TCCR1A |= (1<<WGM10);
  // Vamos utilizar o prescaler de 1024x
  // Para isso, definiremos: CS1(2:0) = 101
  TCCR1B |= (1<<CS12)|(1<<CS10);
  // O valor de comparação será atualizado conforme os valores do seno
  // Resetamos o valor do contador para contar num valor conhecido
  TCNT1 = 0;
  // Configuração de COM1A(1:0) = 01, para operar no seguinte modo:
  // Clear OC1A/OC1B on Compare Match when upcounting.
  // Set OC1A/OC1B on Compare Match when downcounting.
  TCCR1A |= (1<<COM1A1);
  // Para habilitar a interrupção sempre que o contador decrescente chegar no valor minimo
  TIMSK1 |= (1<<TOIE1);
  }

// --------------------------------------------------------------------------

// ---- Função para ativar a saída PWM ----

void ligaPWM(){
// Definimos a porta digital 9 como saída
DDRB |= (1<<DDB1);
}

// --------------------------------------------------------------------------

// ---- Função para desativar a saída PWM ----

void desligaPWM(){
// Definimos a porta digital 9 como entrada
DDRB &= ~(1<<DDB1);}

// --------------------------------------------------------------------------

// ---- Função para configuração da interrupção externa no pino 2 ----

void confEI(){
  // Configura para ativar a interrupção externa para eventos de borda de subida
  EICRA |= (1<<ISC01) | (1<<ISC00);
  // Interrupção interna na porta do INT0 é ativada 
  EIMSK |=  (1<<INT0); 
  // Configuramos o pino 2 como entrada
  DDRD &= ~((1<<DDD2)); 
}

// --------------------------------------------------------------------------

// ---- Função para a configuração do ADC ----

 //configuracao do ADC
void cfgADC(){
  ADMUX = (1<<REFS0); //Referencia de voltagem  5V REFS 1:0= 01
  // A configuracao acima ja faz a entrada A1 estar selecionada, fazendo REFS 1:0 = 00, a entrada sera A0
  //REFS funciona como um multiplex das entradas
  ADCSRA = (1<<ADPS2)|(1<<ADPS1); //Configurar prescaler
           //em 64x 
           //estava em 128x e operava em 125kHz, dividindo o prescaler por 2, aumentamos para 250kHZ
           //ADPS 2:0 = 110
 ADCSRB = 0; //Captura livre
 ADCSRA |= (1<<ADEN)| (1<<ADIE); //Habilita ADC ADEN e interrupcao
}

// --------------------------------------------------------------------------

// ---- Função que faz o mapeamento dos valores do ADC para valores de PWM ---- 

int adc2Pwm (int val0 ,int val1){
  long out;
  int pwm_val;
  int ver1, ver0;
  
  // Verficação se val0 está no intervalo de valores permitidos
  if ((( val1 - (128)) < val0) && (val0 < (val1 +(128)))){
  ver0 = 1;}
  else {
  ver0 = 0;}
  
  // Verificação se val1 está no intervalo de valores permitidos
  if (((val0 - (128)) < val1) && (val1 < (val0 + (128)))){
  ver1 = 1;
  }
  else {
  ver1 = 0;}
  
  // Apenas será verdade quando os dois valores estiverem no intervalo permitido
  if (ver0 && ver1){
  out = (val0 + val1)>>1;}
  else {
  out = 0;}
  
  // Saída normalizada para os valores entre 0 e 255
  pwm_val = out>>2;
  return pwm_val;
}

// --------------------------------------------------------------------------

// ---- Funções para controle do sentido de rotação do motor com a ponte H ----

void horario(){
DDRD |= ((1<<DDD7)|(1<<DDD6));
PORTD |= ((1<<PD7));
PORTD &= (~(1<<PD6));
}

void antiHorario(){
DDRD |= ((1<<DDD7)|(1<<DDD6));
PORTD &= ~((1<<PD7));
PORTD |= ((1<<PD6));
}

void pontoMorto(){
DDRD |= ((1<<DDD7)|(1<<DDD6));
PORTD &= ((1<<PD7)|(1<<PD6));
}

void freio(){
DDRD |= ((1<<DDD7)|(1<<DDD6));
PORTD |= ((1<<PD7)|(1<<PD6));
}

// --------------------------------------------------------------------------

// ---- Variaveis volatile que serão utilizadas ---- 

volatile int pulsos; // Indicará a quantidade de pulsos
volatile long vel = 0; // Indicará a velocidade medida
volatile unsigned long mult = 0;
volatile unsigned int pulsos_por_volta = 40;
volatile unsigned int kt = 1875;

volatile byte disp0 = 0, disp1 = 0; //variaveis que dirao se o adc ja esta pronto para ser transmitido para o serial
volatile int valor0 = 0;
volatile int valor1 = 0; //variaveis que armazenarao o valor medido no adc

// --------------------------------------------------------------------------

// ---- Função que implementa o controlador PID ---- 

volatile int setpoint; // = pwm2Vel(adc2Pwm(valor0,valor1)); // Armenará o PWM resultante da leitura dos ADCs
volatile int PID = 0;
volatile int erro = 0;
volatile int erro_pre = 0;
volatile long erro_sum = 0;
volatile int erro_sub = 0;
volatile int conta;


void cPID(){
  
  erro = (setpoint - vel);
  PID = (erro>>1) + (erro_sum>>3) + (erro_sum>>4) + (erro_sub>>3) + (erro_sub>>4);
  // Analisar esse if para controle do sentido de rotação
  if ((PID > 0) && (PID<2084)){ // Girar no sentido horario
  horario();
  PID = PID;}
  else if(PID > 2084){
    PID = 2084;}
  else { // Girar no sentido anti-horario
    //antiHorario();
    PID = 0;}
  erro_pre = erro;
  erro_sub = erro - erro_pre;
  erro_sum = erro + erro_sum;
  if (erro_sum >20000) {erro_sum = 20000;}
  if (erro_sum <-20000) {erro_sum = -20000;}
}

// --------------------------------------------------------------------------

// ---- Função que indica quando a leitura dos ADCs já foi concluida ---- 

void trADC()
  {
  if(disp0){
    //Serial.println(valor0);
    disp0 = 0;
  }
  if(disp1){
    //Serial.println(valor1);
    disp1 = 0;
  }
  }

// --------------------------------------------------------------------------

// ---- Rotina de interrupção para a interrupção externa no pino digital 2 ----

ISR(INT0_vect){
  // Desativamos a interrupção externa
  // (Debouncing) Damos um pequeno delay para evitar trepidações
  EIMSK &= ~(1<<INT0);
  delayMicroseconds(50);
  pulsos++;
  // Reativamos a interrupção externa
  EIMSK |= (1<<INT0);
}

// --------------------------------------------------------------------------

// ---- Rotina de interrupção em intervalos de 32 mseg ---- 

ISR(TIMER1_OVF_vect){
cli();
vel = ((long)kt*(long)pulsos)/pulsos_por_volta; // Calculamos o valor da velocidade
setpoint = pwm2Vel(adc2Pwm(valor0,valor1)); // Calcula a velocidade requisitada
cPID(); // Executamos a função cPID() para calcular o novo valor do PWM
OCR1A = vel2Pwm(PID);
//vel2Pwm(PID); // Atribuimos o valor do PWM resultante do controlador PID
pulsos = 0;
conta++;
ADMUX  &= ~(1<<MUX0); //reseta ADMUX, fazendo com que a porta lida seja a A0
ADCSRA |= (1<<ADSC);  //adsc e o bit que inicializa a conversao AD
sei();
}

// --------------------------------------------------------------------------

// ---- Rotina de interrupção do ADC ---- 
volatile int erro_adc;
ISR(ADC_vect){ //Ocorre na conclusao da conversao
cli();
  //logica para multiplexar as entradas A0 e A1
  if((ADMUX &(1<<MUX0)) == 0)  //ve se o bit do multiplex esta em 0, se sim:
  {
    valor0 = ADC;       //le o valor que esta no ADC e salva em valor0
    ADMUX |= (1<<MUX0); //seta o lsb de ADMUX em 1 para posteriormente ler o valor da porta A1
    ADCSRA |= (1<<ADSC);  //adsc e o bit que inicializa a conversao AD | gera dnv a interrupcao
    disp0 = 1;           //avisa  que terminou (sera importante no void loop()
   }
   else      //caso contrario, se o lsb de ADMUX estiver em 1, ou seja, a porta selecionada e a A1
   {
    valor1 = ADC;   //le o valor
    erro_adc = valor0 - valor1; // calculo do erro dos adcs
    disp1 = 1;      //avisa que terminou
    }
sei();
}

// --------------------------------------------------------------------------
