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

int currentInc;
int anInc = 5;
static int state;
Timer fadeTimer(MICROS);
Timer sleepTimer(MILLIS);


void setup()
{
  Serial.begin(9600);
  pinMode(Led_1, OUTPUT);
  pinMode(Led_2, OUTPUT);
  pinMode(Led_3, OUTPUT);
  pinMode(Led_4, OUTPUT);
  
  noInterrupts();  
  state = 0;
  interrupts();
  
  //currentInc = 0;
  fadeTimer.start();
  //Timer1.initialize();
  //Timer1.attachInterrupt(redFade, Fade_Timer);
  sleepTimer.start();

}

void loop()
{
  swtch(state){
    case 0:
      redFade();
      sleep();  
      break;    
    case 1:
      pattern();
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

void wakeUp(){
  
}

void sleep()
{
  if(sleepTimer.read() >= Sleep_Timer){
    Serial.println("sleep");
    //pinMode(But_1,INPUT);
    enableInterrupt(But_1, wakeUp, CHANGE);
  //analogWrite(Led_R, 0);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
/** The program will continue from here. **/
  Serial.println("WAKE UP");
/* First thing to do is disable sleep. */
  Serial.println("call setup");
  sleep_disable();
  disableInterrupt(But_1);
  setup();
  }
}

void pattern()
{
  int arr[4] = {0};
  randomSeed(analogRead(A5));
  for(int i=0; i<4;i++){
    arr[i] = random(2);  
    //Serial.print(arr[i]);
  }
  digitalWrite(Led_1, arr[0]);
  digitalWrite(Led_2, arr[1]);
  digitalWrite(Led_3, arr[2]);
  digitalWrite(Led_4, arr[3]);  
  //Serial.println("-------------------------");
}
