//#include <TimerOne.h>
#include <arduino-timer.h>
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
#define PENALTY 4

#define MAX_PENALITY 3

#define FadeTime 20
#define WaitTime 10000
#define T2 10000

/*Timer fadeTimer(MILLIS);
Timer sleepTimer(MILLIS);     
//Timer patternTimer(MILLIS);
Timer playTimer(MILLIS);
Timer penaltyTimer(MICROS); */

Timer<2> startTimer;
//Timer patternTimer;

int T1;           //tempo random prima del pattern
float F;          //fattore di riduzione tempo
int currentInc;   //valore analogico RedLed
int anInc = 5;    //incremento valore analogico
int state;        //stato del gioco
int pens;         //numero di penalità
int score;        //punteggio
int currentPot;   //potenza precedente del potenziometro


bool ledPattern[4] = {0};
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

  startTimer.every(FadeTime, redFade);
  startTimer.every(WaitTime, sleep);
}

void setLed(int l1, int l2, int l3, int l4){
    digitalWrite(Led_1,l1);
    digitalWrite(Led_2,l2);
    digitalWrite(Led_3,l3);
    digitalWrite(Led_4,l4);
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

bool redFade(){

    analogWrite(Led_R, currentInc);
    if(currentInc + anInc < 0 || currentInc + anInc > 255){
      anInc = -anInc;
    }  
    currentInc += anInc;
    return true;
}

bool sleep()
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
  return true;
}

bool pattern(){

  Serial.println("pattern");
  randomSeed(analogRead(A5));
  for(int i=0; i<4;i++){
    ledPattern[i] = random(2);  
  }
  setLed(ledPattern[0],ledPattern[1],ledPattern[2],ledPattern[3]);

  return true; 
}

void penalty(){
  
    Serial.println("PENALTY!!!");
    digitalWrite(Led_R, HIGH);
    
    noInterrupts();
    pens++;
    state = PENALTY;
    interrupts();     

    startTimer.cancel();
    startTimer.in(1000, offPenalty);
    
    for(int i=0;i<4;i++){
      playerPattern[i] = 0;     
    }
    setLed(LOW, LOW, LOW, LOW);
}

void offPenalty(){
  
    digitalWrite(Led_R, LOW);

    if(pens < MAX_PENALITY-1){ 

      noInterrupts();
      state = PATTERN;
      interrupts();
      
      T1 = random(5);
      float Ftime = (float)T2/(1+F*score);

      startTimer.cancel();
      startTimer.in(T1*1000, pattern);
      startTimer.in(Ftime + T1*1000, offPenalty);

    } else{
      
      Serial.print("Game Over. Final Score: ");
      Serial.println(score);
      setLed(LOW,LOW,LOW,LOW);
   
      noInterrupts();
      pens = 0;
      score = 0;
      state = WAIT;     
      interrupts();
      
      startTimer.cancel();
      startTimer.every(FadeTime, redFade);
      startTimer.every(WaitTime, sleep);
    }  
}

/*void redOff(){
    noInterrupts();
    state = PATTERN;
    interrupts();

    startTimer.cancel();
    startTimer.in(T1*1000, pattern);
    float Ftime = (float)T2/(1+F*score);
    startTimer.in(Ftime + T1*1000, offPenalty);
}*/



bool playing(){
  Serial.println("Gioca");

  noInterrupts();
  state = PLAY;
  interrupts();

  setLed(LOW,LOW, LOW, LOW);

  startTimer.cancel();
  float Ftime = (float)T2/(1+F*score);

  startTimer.in(Ftime, checkPattern);

  return true;
}



bool checkPattern(){
  
  noInterrupts();
  bool check = true;
  for(int i=0; i<4; i++){
    if(ledPattern[i] != playerPattern[i]){
      check = false;
    }
  }
  Serial.println(check);
  //state = PATTERN;
  interrupts();
  
  setLed(LOW,LOW,LOW,LOW);

  if(check){
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);

    noInterrupts();
    state = PATTERN;
    interrupts();

    T1 = random(5);
    float Ftime = (float)T2/(1+F*score);

    startTimer.cancel();

    Serial.println(startTimer.size());

    startTimer.in(1000, pattern); // Return true
    startTimer.in(3000, playing);

    Serial.println(startTimer.size());

  }else{
    penalty();
  }
  return true;
}



void calledInterrupt(){
   
  unsigned int pin = arduinoInterruptedPin; 
  
  switch (state){
    
    case WAIT:
      if(pin == But_1){
        
        digitalWrite(Led_R, LOW);
        state = PATTERN;

        T1 = random(5);
        float Ftime = (float)T2/(1+F*score);

        startTimer.cancel();
        
        startTimer.in(T1*1000, pattern); // Return true
        startTimer.in(Ftime + T1*1000, playing);
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

void loop() {
  
  switch (state){

    case WAIT:
      startTimer.tick();    
      potLevel();
      break;

    case PATTERN:    
      startTimer.tick();                
      break;

    case PENALTY:
      startTimer.tick();
      break;

    case PLAY:
      startTimer.tick();
      break;

    default:
      break;
  }
}

