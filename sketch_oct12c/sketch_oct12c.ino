#include <TimerOne.h>
#include <Timer.h>
#include <avr/sleep.h>
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

#define Fade_Timer 20000
#define Sleep_Timer 10000
#define BouncingTime 200

int T1;
int currentInc;
int anInc = 5;
int state;

int arr[4] = {0};
int ris[4] = {0};

Timer fadeTimer(MICROS);
Timer sleepTimer(MILLIS);
Timer endTimer(MILLIS);
Timer bounceTimer(MILLIS);

void setup()
{
  Serial.begin(9600);
  pinMode(Led_1, OUTPUT);
  pinMode(Led_2, OUTPUT);
  pinMode(Led_3, OUTPUT);
  pinMode(Led_4, OUTPUT);

  pinMode(But_1, INPUT);
  pinMode(But_2, INPUT);
  pinMode(But_3, INPUT);
  pinMode(But_4, INPUT);
  
  noInterrupts();  
  state = 0;
  T1 = 1;
  interrupts();
  
  fadeTimer.start();
  sleepTimer.start();
  bounceTimer.start();    
}

void loop()
{
  switch(state){      

    case 0:
      redFade();      
      sleep();
      if(digitalRead(But_1) && bounceTimer.read() >= BouncingTime){
        bounceTimer.start();        
        noInterrupts();
        state++;
        interrupts();   
      }       
      break;   

    case 1:
      digitalWrite(Led_R, LOW);
      pattern();
      delay(10000/T1);

      digitalWrite(Led_1, LOW);
      digitalWrite(Led_2, LOW);
      digitalWrite(Led_3, LOW);
      digitalWrite(Led_4, LOW);

      noInterrupts();
      state++;
      interrupts();

      endTimer.start();
      //Timer1.initialize();
      //Timer1.attachInterrupt(endGame, 1000000);
      break;

    case 2:
      digitalWrite(Led_R, HIGH);
      play();   
      if(endTimer.read() >= 10000){
        noInterrupts();
        //state++;
        interrupts();   
      }      
      break;
      
    case 3:
      digitalWrite(Led_R, LOW);   
      break;      
  }
}


void redFade()
{
  if(fadeTimer.read() >= Fade_Timer){
    fadeTimer.start();
    analogWrite(Led_R, currentInc);
    if(currentInc + anInc < 0 || currentInc + anInc > 255){
      anInc = -anInc;
    }  
    currentInc += anInc;
  }
}

void wakeUp(){}

bool sleep()
{
  if(sleepTimer.read() >= Sleep_Timer){

    enableInterrupt(But_1, wakeUp, RISING);
    enableInterrupt(But_2, wakeUp, RISING);
    enableInterrupt(But_3, wakeUp, RISING);
    enableInterrupt(But_4, wakeUp, RISING);

    digitalWrite(Led_R, LOW);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    sleep_mode();
    /** The program will continue from here. **/
    sleep_disable();
    
    disableInterrupt(But_1);
    disableInterrupt(But_2);
    disableInterrupt(But_3);
    disableInterrupt(But_4);
    
    sleepTimer.start();
    fadeTimer.start();
    bounceTimer.start();    


    return true;
  }
  return false;
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
}

void write(int pos){
  if(ris[pos] == 0){
    ris[pos] = 1;
  } else {
    ris[pos] = 0;
  }
}


void play(){

  bool tp=false;
  if(digitalRead(But_1) && bounceTimer.read() >= BouncingTime){
    ris[0] = (ris[0] + 3) % 2;
    digitalWrite(Led_1, ris[0]);  
    tp = true;

  } else if (digitalRead(But_2) && bounceTimer.read() >= BouncingTime){
    ris[1] = (ris[1] + 3) % 2;
    digitalWrite(Led_2, ris[1]);
    tp = true;

  } else if (digitalRead(But_3) && bounceTimer.read() >= BouncingTime){ 
    ris[2] = (ris[2] + 3) % 2;
    digitalWrite(Led_3, ris[2]);  
    tp = true;
      
  } else if (digitalRead(But_4) && bounceTimer.read() >= BouncingTime){
    ris[3] = (ris[3] + 3) % 2;    
    digitalWrite(Led_4, ris[3]);
    tp = true;

  } 
  
  if (tp){
    bounceTimer.start();
  }

}

void endGame(){
  noInterrupts();
  state++;
  interrupts();
}
