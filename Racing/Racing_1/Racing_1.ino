#define R_MARK_SNS_PIN 5
#define L_MARK_SNS_PIN 6
#define R_LINE_SNS_PIN A0
#define L_LINE_SNS_PIN A1
#define R_POW_PIN 10
#define L_POW_PIN 11
#define R_DIR_PIN 12
#define L_DIR_PIN 13
#define FWD LOW
#define BWD HIGH
#define BREAK_LED 3
#define LEFT_LED  8
#define RIGHT_LED 9

void setup() {
    Serial.begin(9600);
    pinMode(R_MARK_SNS_PIN, INPUT);
    pinMode(L_MARK_SNS_PIN, INPUT);
    pinMode(R_LINE_SNS_PIN, INPUT);
    pinMode(L_LINE_SNS_PIN, INPUT);
    pinMode(R_DIR_PIN, OUTPUT);
    pinMode(L_DIR_PIN, OUTPUT);
    pinMode(BREAK_LED, OUTPUT);
    pinMode(LEFT_LED, OUTPUT);
    pinMode(RIGHT_LED, OUTPUT);
    digitalWrite(R_DIR_PIN, FWD);
    digitalWrite(L_DIR_PIN, FWD);
}

int endTrack = 20;                     //돌아야 하는 바퀴 수를 입력합니다. (marking의 개수)
int doTrack = 0;                      //현재 지난 marking의 개수를 기록합니다.
boolean markIn = false;               //marking을 지나고 있는지 체크합니다.
boolean prevMarkIn = false;

double k = 0.93;                       //sensor의 값에 따른 pow의 민감도

double maxSensor = 350.0;              //sensor의 최대값 
double minSensor = 25.0;              //sensor의 최소값

int maxSpeed = 255;

double motorStep = 310.0 / maxSensor; //sensor값에 따른 motor의 민감도

boolean isMark = false;               //한쪽이 mark를 지닐 때 체크합니다.

int prevTime;
bool isConer = false;

bool isBlink = false;
long blinkTime = 0;
int blinkTarget;
int breakPower = 0;

void ledControl(double rPowValue, double lPowValue){
  if(rPowValue >= (lPowValue + 2)){
    /* 좌회전 깜빡이 */
    blinkTime = 500 - (rPowValue - lPowValue);
    if(prevTime + blinkTime >= millis()){
      isBlink = !isBlink;
      digitalWrite(LEFT_LED, isBlink);
      prevTime = millis();
    }
    digitalWrite(RIGHT_LED, LOW);
  } else if(lPowValue >= (rPowValue + 2)){
    /* 우회전 깜빡이 */
    blinkTime = 500 - (rPowValue - lPowValue);
    if(prevTime + blinkTime >= millis()){
      isBlink = !isBlink;
      digitalWrite(RIGHT_LED, isBlink);
      prevTime = millis();
    }
    digitalWrite(LEFT_LED, LOW);
  } else{
    isBlink = false;
    digitalWrite(LEFT_LED, LOW);
    digitalWrite(RIGHT_LED, LOW);
    prevTime = millis();
  }

  breakPower = 500 - (rPowValue + lPowValue);
  if(breakPower < 0)  breakPower = 0;
  analogWrite(BREAK_LED, breakPower);
}

void loop() {
    if (doTrack < endTrack) {
        // 센서 값을 불러옵니다.
        int rms = digitalRead(R_MARK_SNS_PIN);    //마크 신호값을 받습니다.(0, 1)
        int lms = digitalRead(L_MARK_SNS_PIN);
        int inLine = (rms && lms);

        int rLine = analogRead(R_LINE_SNS_PIN);   //라인 신호값을 받습니다.(0 ~ 980)
        int lLine = analogRead(L_LINE_SNS_PIN);
        if (rLine >= maxSensor) rLine = maxSensor;
        if (lLine >= maxSensor) lLine = maxSensor;

        if (inLine && !prevMarkIn) markIn = true;
        else if (!inLine && prevMarkIn) {
            prevMarkIn = false;
        }

        if (markIn) {
            doTrack++;
            markIn = false;
            prevMarkIn = true;
        }

        /* pow값을 계산합니다. mark를 지날때는 이전 motor값을 유지합니다. */
        double rPowValue = motorStep * (maxSensor - rLine * k);
        double lPowValue = motorStep * (maxSensor - lLine * k);
        if (rPowValue >= maxSpeed) rPowValue = maxSpeed;
        if (lPowValue >= maxSpeed) lPowValue = maxSpeed;

        ledControl(rPowValue, lPowValue);

        analogWrite(L_POW_PIN, lPowValue);
        analogWrite(R_POW_PIN, rPowValue);
    }
    else {
        analogWrite(R_POW_PIN, 0);
        analogWrite(L_POW_PIN, 0);
    }
}
