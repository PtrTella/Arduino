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
#define PlayTime 10000

Timer fadeTimer(MILLIS);
Timer sleepTimer(MILLIS);
Timer patternTimer(MILLIS);
Timer playTimer(MILLIS);

int T1;     //tempo random prima del pattern
int F;  //fattore di riduzione tempo
int currentInc;
int anInc = 5;
int state;
int pens;   //numero di penalità
int ran;
int score;


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
  F = 1;
  Serial.println("Welcome to the Catch the Led Pattern Game. Press Key T1 to Start");
  fadeTimer.start();
  sleepTimer.start();
  interrupts();
}

void setLed(int l1, int l2, int l3, int l4){
    digitalWrite(Led_1,l1);
    digitalWrite(Led_2,l2);
    digitalWrite(Led_3,l3);
    digitalWrite(Led_4,l4);
}

void penalty(){
  if(pens < MAX_PENALITY-1){
    Serial.println("PENALTY!!!");
    pens++;
  } else{
    Serial.println("Game Over. Final Score: %d" + score);
    pens = 0;
    score = 0;
    state = WAIT;
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
    ledPattern[i] = random(2);  
  }
  setLed(ledPattern[0],ledPattern[1],ledPattern[2],ledPattern[3]);
  patternTimer.start();
  while(patternTimer.read() <= PlayTime/F && state == PATTERN) {}
  if(state == PATTERN){
    setLed(LOW,LOW,LOW,LOW);
    Serial.println("Go!");
    playTimer.start();
    state = PLAY;
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
    
    case WAIT: //waiting
      if(pin == But_1){
        state = PATTERN;
        T1 = random(5);
        patternTimer.start();
      }     
      break;

    case PATTERN: //pattern
      penalty();
      break;

    case PLAY:
         
      switch (pin){
        case But_1:
          playerPattern[0] = !playerPattern[0];
          digitalWrite(Led_1, playerPattern[0]);  
          break;
          
        case But_2:
          playerPattern[1] = !playerPattern[1];
          digitalWrite(Led_2, playerPattern[1]);     

        case But_3:
          playerPattern[2] = !playerPattern[2];
          digitalWrite(Led_3, playerPattern[2]); 
          break;

        case But_4:
          playerPattern[3] = !playerPattern[3];    
          digitalWrite(Led_4, playerPattern[3]);      
          break;
        }
      break;    
    }
}

void checkPattern(){
  bool check = true;
  for(int i=0; i<4; i++){
    if(ledPattern[i] != playerPattern[i]){
      check = false;
      break;
    }
  }
  state = PATTERN;
  if(check){
    score++;
    Serial.println("New point! Score: &d" + score);
  }else{
    penalty();
  }
  patternTimer.start();
}

void loop() {
  
  switch (state){

      case WAIT:
        redFade();
        if(sleepTimer.read() >= SleepTime){
          sleep();
          sleepTimer.start();
          //fadeTimer.start();   non serve perchè quando richiamo fade timer con un tempo maggiore ci pensa la redFade a farlo riniziare
        }
        break;

      case PATTERN:    
        if(patternTimer.read() >= T1*1000){
          pattern();
        }                                
        break;

      case PLAY:
        if(playTimer.read() >= PlayTime){
          checkPattern();
        }
        break; 
      }

}
