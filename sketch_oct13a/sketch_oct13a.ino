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
#define SleepTime 10000
#define BouncingTime 200

Timer fadeTimer(MILLIS);
Timer sleepTimer(MILLIS);
Timer patternTimer(MILLIS);

int T1;
int currentInc;
int anInc = 5;
int state;
int pens;
int ran;


int arr[4] = {0};
bool ris[4] = {0};

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
  state = 1;
  pens = 0;
  interrupts(); 

  
  fadeTimer.start();
  sleepTimer.start();
}

void penalty(){
  // INCREMENTA PENALITA E CONTROLLA QUNATE NE HO
  if(pens < MAX_PENALITY-1){
    Serial.println("PENALTY!!!");
    pens++;
  } else{
    // STAMPA DEL FINE GIOCO
    Serial.println("END GAME!!!");
    pens = 0;
    state = WAIT;
    digitalWrite(Led_1,LOW);
    digitalWrite(Led_2,LOW);
    digitalWrite(Led_3,LOW);
    digitalWrite(Led_4,LOW);
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
    /** The program will continue from here. **/
    sleep_disable();
    noInterrupts();
    state = WAIT;
    interrupts(); 
}

void pattern()
{
  randomSeed(analogRead(A5));
  for(int i=0; i<4;i++){
    arr[i] = random(2);  
  }
  digitalWrite(Led_1, arr[0]);
  digitalWrite(Led_2, arr[1]);
  digitalWrite(Led_3, arr[2]);
  digitalWrite(Led_4, arr[3]);  
  patternTimer.start();
  while(patternTimer.read() <= 20000 && state == PATTERN) {}
  if(state == PATTERN)
    state = PLAY;
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
    
    case WAIT: //waiting
      if(pin == But_1){
        state = PATTERN;
        patternTimer.start();
      }     
      break;

    case PATTERN: //pattern
      penalty();
      break;

    case PLAY:
         
      switch (pin){
        case But_1:
          ris[0] = !ris[0];
          digitalWrite(Led_1, ris[0]);  
          break;
          
        case But_2:
          ris[1] = !ris[1];
          digitalWrite(Led_2, ris[1]);     

        case But_3:
          ris[2] = !ris[2];
          digitalWrite(Led_3, ris[2]); 
          break;

        case But_4:
          ris[3] = !ris[3];    
          digitalWrite(Led_4, ris[3]);      
          break;
        }
      break;    
    }
}

void goPlay(){
  noInterrupts();
  state = PLAY;
  interrupts();

  //LED SPENTI TUTTI
}

void loop() {
  
  switch (state){

      case WAIT:
        redFade();
        if(sleepTimer.read() >= SleepTime){
          sleep();
          sleepTimer.start();
          fadeTimer.start();
        }
        break;

      case PATTERN:    
        if(patternTimer.read() >= 1000){
          pattern();
        }                                
        break;

      case PLAY:
        
        break; 
      }
  

}
