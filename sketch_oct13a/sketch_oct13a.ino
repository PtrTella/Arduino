#include <TimerOne.h>
#include <Timer.h>
#include <avr/sleep.h>
#define EI_ARDUINO_INTERRUPTED_PIN
#include <EnableInterrupt.h>

#define Led_1 13
#define Led_2 11
#define Led_3 9
#define Led_4 7

#define Led_R 5 

#define But_1 12
#define But_2 10
#define But_3 8
#define But_4 6

#define Pot A0

#define SLEEP 0
#define WAIT 1
#define PATTERN 2
#define PLAY 3
#define END 4

#define MAX_PENALITY 3

#define FadeTime 20
#define WaitTime 10000
#define T2 10000

Timer fadeTimer(MILLIS);
Timer sleepTimer(MILLIS);
Timer patternTimer(MILLIS);
Timer playTimer(MILLIS);
Timer penaltyTimer(MICROS);

int T1;           //tempo random prima del pattern
float F;          //fattore di riduzione tempo
int currentInc;   //valore analogico RedLed
int anInc = 5;    //incremento valore analogico
int state;        //stato del gioco
int pens;         //numero di penalità
int ran;          //tempo random prima di accendere il pattern
int score;        //punteggio
int currentPot;   //potenza precedente del potenziometro


int ledPattern[4] = {0};
bool playerPattern[4] = {0};

void setup()
{
  Serial.begin(9600);

  pinMode(Led_1, OUTPUT);
  pinMode(Led_2, OUTPUT);
  pinMode(Led_3, OUTPUT);
  pinMode(Led_4, OUTPUT);

  enableInterrupt(But_1, calledInterrupt, RISING);
  enableInterrupt(But_2, calledInterrupt, RISING);
  enableInterrupt(But_3, calledInterrupt, RISING);
  enableInterrupt(But_4, calledInterrupt, RISING);
  
  noInterrupts();  
  state = WAIT;
  pens = 0;
  score = 0;
  currentPot = 0;
  F = 0,20;
  interrupts();

  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
  fadeTimer.start();
  sleepTimer.start();
  //penaltyTimer.start();
}

void setLed(int l1, int l2, int l3, int l4){
    digitalWrite(Led_1,l1);
    digitalWrite(Led_2,l2);
    digitalWrite(Led_3,l3);
    digitalWrite(Led_4,l4);
}

void penalty(){
  if(pens < MAX_PENALITY-1){   
    //digitalWrite(Led_R, HIGH);
    Serial.println("PENALTY!!!");
    //while(penaltyTimer.read() <= 1000000) {Serial.println(penaltyTimer.read());}
    noInterrupts();
    pens++;
    interrupts();    
    //digitalWrite(Led_R, LOW);    
  } else{
    Serial.print("Game Over. Final Score: ");
    Serial.println(score);
    noInterrupts();
    pens = 0;
    score = 0;
    state = WAIT;     
    interrupts();
    setLed(LOW,LOW,LOW,LOW);
    fadeTimer.start();
    sleepTimer.start();
  }
}


void sleep()
{
  digitalWrite(Led_R, LOW);
  noInterrupts();
  state = SLEEP;
  interrupts();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();
    
  noInterrupts();
  state = WAIT;
  interrupts(); 
}

void pattern()
{
  randomSeed(analogRead(A5));
  for(int i=0; i<4;i++){
    ledPattern[i] = random(2);  
  }
  setLed(ledPattern[0],ledPattern[1],ledPattern[2],ledPattern[3]);
  patternTimer.start();
  
  float Ftime = (float)T2/(1+F*score);
  Serial.println(Ftime); 
  while(patternTimer.read() <= Ftime && state == PATTERN) {}
  
  if(state == PATTERN){
    for(int i=0;i<4;i++){
      playerPattern[i] = 0;     
    }
    setLed(LOW,LOW,LOW,LOW);
    Serial.println("Go!");
    playTimer.start();
    
    noInterrupts();
    state = PLAY;
    interrupts();
  }
}

void redFade()
{
  if(fadeTimer.read() >= FadeTime){
    fadeTimer.start();
    analogWrite(Led_R, currentInc);
    if(currentInc + anInc < 0 || currentInc + anInc > 255){
      anInc = -anInc;
    }  
    currentInc += anInc;
  }
}

void calledInterrupt(){
   
  unsigned int pin = arduinoInterruptedPin; 
  
  switch (state){
    
    case WAIT:
      if(pin == But_1){
        state = PATTERN;
        digitalWrite(Led_R, LOW);
        T1 = random(5);
        patternTimer.start();
      }
      break;

    case PATTERN:
      penalty();
      break;

    case PLAY:
      switch (pin){
        case But_1:
          //Serial.println("B1 pressd");
          playerPattern[0] = !playerPattern[0];
          digitalWrite(Led_1, playerPattern[0]);  
          break;
          
        case But_2:
          //Serial.println("B2 pressd");
          playerPattern[1] = !playerPattern[1];
          digitalWrite(Led_2, playerPattern[1]);
          break;     

        case But_3:
          //Serial.println("B3 pressd");
          playerPattern[2] = !playerPattern[2];
          digitalWrite(Led_3, playerPattern[2]); 
          break;

        case But_4:
          //Serial.println("B4 pressd");
          playerPattern[3] = !playerPattern[3];    
          digitalWrite(Led_4, playerPattern[3]);      
          break;
        }
      break; 
    }
}

void checkPattern(){
  
  noInterrupts();
  bool check = true;
  for(int i=0; i<4; i++){
    if(ledPattern[i] != playerPattern[i]){
      check = false;
      break;
    }
  }
  state = PATTERN;
  interrupts();
  
  setLed(LOW,LOW,LOW,LOW);
  if(check){
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);
  }else{
    penalty();
  }
  patternTimer.start();
}

void potLevel(){
  int newValue = analogRead(Pot);
  if(newValue < currentPot-255 || newValue > currentPot+255){
    currentPot = newValue;  
    int val = currentPot/256 + 1;
    F = (float)val/5;
    Serial.print("Level: ");
    Serial.println(val);
  }
}

void loop() {
  
  switch (state){

    case WAIT:
      redFade();
      potLevel();
      if(sleepTimer.read() >= WaitTime){
        sleep();
        sleepTimer.start();
      }
      break;

    case PATTERN:    
      if(patternTimer.read() >= T1*1000){
        pattern();
      }                                
      break;

    case PLAY:
      if(playTimer.read() >= (T2/(1+F*score)) ){     
        checkPattern();
      }
      break;
        
    default:
      break;
  }
}
