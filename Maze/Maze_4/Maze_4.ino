#include <SoftwareSerial.h>
#include <LedControl.h>

LedControl dot = LedControl(7, 6, 5, 1);

#define R_SONIC A0
#define L_SONIC A1
#define F_SONIC A2
#define L_MOTOR 11
#define R_MOTOR 10
#define L_DIR 13
#define R_DIR 12
#define MAX_SPEED 255
#define MIN_SPEED 0
#define FWD LOW
#define BWD HIGH

#define BTN1 4
#define BTN2 8

#define TREE1 3
#define TREE2 9

#define MAX_VALUE 110                                      // 센서 최대 값
#define JUDGE_VALUE 60                                     // 센서 판정 값
#define MARGIN_VALUE 20                                     // 센서 마진

#define DELAY_TIME 1500
#define LEFT_DELAY_TIME 1000

#define FRONT_LED 2
#define LEFT_LED  3
#define RIGHT_LED 4
#define BACK_LED  5
#define M_STEP1   6
#define M_STEP2   7

byte none[] = {
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000
};

byte leftArrow[] = {
  B00010000,
  B00110000,
  B01100000,
  B11111111,
  B11111111,
  B01100000,
  B00110000,
  B00010000
};

byte rightArrow[] = {
  B00001000,
  B00001100,
  B00000110,
  B11111111,
  B11111111,
  B00000110,
  B00001100,
  B00001000
};

byte backArrow[] = {
  B00011100,
  B00100010,
  B00100010,
  B00100010,
  B10101010,
  B01110010,
  B00100010,
  B00000010
};

byte straightArrow[] = {
  B00011000,
  B00111100,
  B01111110,
  B11011011,
  B00011000,
  B00011000,
  B00011000,
  B00011000
};

long dotIndex = 0;

int step = 0;                                                         // 동작 스탭(0: 직진 및 우회전, 1: 좌회전, 3: 유턴)
int substep = 0;                                                    // 회전 스탭(0: 회전중이 아님, 1: 직진 주행, 2: 회전 주행, 3: 회전 후 안정화 주행)
int leftSensor, rightSensor, frontSensor;
int lPowValue, rPowValue;

long currTime, treeTime;

int sensorValue;
int forwardSensorMax = JUDGE_VALUE + MARGIN_VALUE;
int forwardSensorMin = JUDGE_VALUE - MARGIN_VALUE;
double rightValue, leftValue;
double forwardMax = 390;
double forwardMin = 160;

bool turnYn = false;
bool treeYn = false;
int treeIndex = 0;

/* 직진 운동 */
void forward() {
    /* 직진 운동을 위한 센서 값 보정 */
    sensorValue = rightSensor > forwardSensorMax ? forwardSensorMax : rightSensor;
    sensorValue = sensorValue < forwardSensorMin ? forwardSensorMin : sensorValue;
    sensorValue -= forwardSensorMin;

    /* 센서 비율 측정 */
    rightValue = ((double)sensorValue / (double)(forwardSensorMax - forwardSensorMin));

    lPowValue = forwardMin + (forwardMax - forwardMin) * rightValue;
    rPowValue = forwardMin + (forwardMax - forwardMin) * (1.0 - rightValue);

    if (rightSensor >= MAX_VALUE) {
        lPowValue *= 1.8;
        rPowValue *= 0.3;
    } else if(rightSensor <= MAX_VALUE && leftSensor <= MAX_VALUE){
      lPowValue *= 1.5;
      rPowValue *= 1.5;
    }
}

void leftTurn() {
    lPowValue = 120;
    rPowValue = 255;  

    if(currTime + LEFT_DELAY_TIME <= millis())  step = 0;
}

/* 유턴 */
void uTurn() {
    lPowValue = 255;
    rPowValue = 255;

    if (currTime + DELAY_TIME <= millis())      step = 0;
}

void dotControl(){
  if(step == 0)       dot.setRow(0, dotIndex, straightArrow[dotIndex]);
  else if(step == 1)  dot.setRow(0, dotIndex, leftArrow[dotIndex]);
  else if(step == 2)  dot.setRow(0, dotIndex, backArrow[dotIndex]);
  else if(step == 3)  dot.setRow(0, dotIndex, rightArrow[dotIndex]);

  dotIndex = (dotIndex + 1) % 8;
}

void treeControl(){
  if(treeYn){
    digitalWrite(TREE1, HIGH);
    digitalWrite(TREE2, LOW);
  } else{
    digitalWrite(TREE1, LOW);
    digitalWrite(TREE2, HIGH);
  }

  if(treeTime + 400 <= millis()){
    treeYn = !treeYn;
    treeTime = millis();
  }
}

void judgeSensor() {
    currTime = millis();
    if ((leftSensor <= MAX_VALUE && rightSensor <= MAX_VALUE) && frontSensor <= MAX_VALUE) {
        /* 모든 센서가 막혀 있을 경우, 유턴으로 설정합니다. */
        step = 2;
        Serial.println("U-Turn");
    } else if ((leftSensor >= MAX_VALUE && frontSensor <= MAX_VALUE + 20)) {
        /* 좌측 센서만 뚫려있는 경우, 좌회전으로 설정합니다. */
        step = 1;
        Serial.println("left-Turn");
    } else {
        step = 0;
    }
}

void setup() {
    Serial.begin(9600);
    pinMode(L_SONIC, INPUT);
    pinMode(R_SONIC, INPUT);
    pinMode(L_DIR, OUTPUT);
    pinMode(R_DIR, OUTPUT);
    pinMode(BTN1, INPUT);
    pinMode(BTN2, INPUT);
    pinMode(TREE1, OUTPUT);
    pinMode(TREE2, OUTPUT);
    digitalWrite(L_DIR, FWD);
    digitalWrite(R_DIR, FWD);

    /* 도트 매트릭스 설정 */
    dot.shutdown(0, false);
    dot.setIntensity(0, 5);
    dot.clearDisplay(0);

    treeTime = millis();
}

void loop() {
    /* 센서 값을 측정합니다 */
    leftSensor = analogRead(L_SONIC);
    rightSensor = analogRead(R_SONIC);
    frontSensor = analogRead(F_SONIC);

    if(step == 1)  leftTurn();
    else if(step == 2)      uTurn();
    else if(step == 0){
        judgeSensor();
        forward();      
    }                

    dotControl();

    treeControl();
    
    if (step == 2 || step == 1)   digitalWrite(L_DIR, BWD);
    else                          digitalWrite(L_DIR, FWD);
    
    analogWrite(L_MOTOR, lPowValue > MAX_SPEED ? MAX_SPEED : lPowValue);
    analogWrite(R_MOTOR, rPowValue > MAX_SPEED ? MAX_SPEED : rPowValue);
}
