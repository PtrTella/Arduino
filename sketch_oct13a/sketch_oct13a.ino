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
#define PENALTY 4
#define END 5

#define MAX_PENALITY 3

#define FadeTime 20
#define WaitTime 10000
#define T2 6000   // Tempo di visibilità del pattern e tempo di gioco, poi scalato con un fattore F

Timer fadeTimer(MILLIS);
Timer timer(MILLIS);

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

void setup(){

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
  timer.start();
}

void setLed(int l1, int l2, int l3, int l4){

    digitalWrite(Led_1,l1);
    digitalWrite(Led_2,l2);
    digitalWrite(Led_3,l3);
    digitalWrite(Led_4,l4);
}

void penalty(){

  setLed(LOW,LOW,LOW,LOW);    
  digitalWrite(Led_R, HIGH);
  Serial.println("PENALTY!!!");
  noInterrupts();
  pens++;
  interrupts();
  delay(1000);
  digitalWrite(Led_R, LOW);  
  if(pens < MAX_PENALITY){
    T1 = random(5);
    state = PATTERN;
    timer.start();                                //start for random time before show pattern
  }else{
    timer.stop();
    state = END;
  }
}

void endGame(){

  digitalWrite(Led_R, HIGH);
  Serial.print("Game Over. Final Score: ");
  Serial.println(score);
  delay(10000);
  digitalWrite(Led_R, LOW);
  noInterrupts();
  pens = 0;
  score = 0;
  state = WAIT;
  interrupts();
  fadeTimer.start();
  timer.start();                                      //start for sleep  
}


void sleep(){

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

void pattern(){

  randomSeed(analogRead(A5));
  for(int i=0; i<4;i++){
    ledPattern[i] = random(2);  
  }
  setLed(ledPattern[0],ledPattern[1],ledPattern[2],ledPattern[3]);
  timer.start();                                  //start to show pattern
  
  float Ftime = (float)T2/(1+F*score);
  while(timer.read() <= Ftime && state == PATTERN) {}
  
  noInterrupts(); 
  if(state == PATTERN){
    for(int i=0;i<4;i++){
      playerPattern[i] = 0;     
    }
    setLed(LOW,LOW,LOW,LOW);
    Serial.println("Gioca!");
    timer.start();                              //start for playing
    state = PLAY;
  }
  interrupts();
}

void redFade(){

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
        Serial.println("Go!");
        state = PATTERN;
        digitalWrite(Led_R, LOW);
        T1 = random(5);
        fadeTimer.stop();
        timer.start();                      //start for random time before show pattern
      }
      break;

    case PATTERN:
      state = PENALTY;
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
          break;     

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
    setLed(LOW,LOW,LOW,LOW);
    score++;
    Serial.print("New point! Score: ");
    Serial.println(score);
  }else{
    state = PENALTY;
  }
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
      if(timer.read() >= WaitTime){
        sleep();
        timer.start();
      }
      break;

    case PATTERN:    
      if(timer.read() >= T1*1000){
        pattern();
      }                       
      break;

    case PLAY:
      if(timer.read() >= (T2/(1+F*score)) ){     
        noInterrupts();
        checkPattern();
        T1 = random(5);
        timer.start();                            //start for random time before show pattern
        interrupts();
      }
      break;
    
    case PENALTY:
      penalty();    
      break;
    
    case END:
      endGame();
      break;

    default:
      break;
  }
}
