#include <Stepper.h>
#include <SoftwareSerial.h>

#define IN1  7 // 8
#define IN2  6 // 9
#define IN3  5 // 10
#define IN4  10 // 11

// Defines the number of steps per rotation
const int stepsPerRevolution = 2038;
//const int stepsPerRevolution = 4095;

#define STEPPER_MAX_SPEED 15

// Creates an instance of stepper class
// Pins entered in sequence IN1-IN3-IN2-IN4 for proper step sequence
Stepper myStepper = Stepper(stepsPerRevolution, IN1, IN3, IN2, IN4);


#define verticalMotorPIN 3 // 5
#define upDownMotorPIN 9 // 6


const int wypelnienieDown = 2;
const int wypelnienieStop = 0;
//lub 3 albo coś małego -> w jedną stronę
const int wypelnienieUp = 253;
//lub 252 albo coś dużego -> w drugą stronę


//wyliczone wartosci dla wypelnienie = 2 wynoszą 1cm / 5s dla silnika serwo
//odległość
const float serwoCmPM = 45.f * 0.1f * 5.f / 6.f; // / 5/6 dobrane eksperymentalnie. Miał przejechać 5 cm, przejechał 6

//policzone dla speed = 15
const float stepperCmPM = 3.925e-4f;

const float stepperCmPerStep = 0.8f / stepsPerRevolution;

void delayMinutes(float minutes) {
  delay(minutes * 60000);
}

float getDelayVerticalMovement(float dst) {
  //czekaj aż silnik serwo skończy pracować
  float delayMinutes = dst / serwoCmPM;
  return delayMinutes;
}

void moveVertical(float dst, boolean doDelay = true) {
  if (dst > 0)
    analogWrite(verticalMotorPIN, wypelnienieUp);
  else if (dst < 0)
    analogWrite(verticalMotorPIN, wypelnienieDown);
  if (doDelay) {
    delayMinutes(getDelayVerticalMovement(abs(dst)));
    analogWrite(verticalMotorPIN, wypelnienieStop);
  }
}

void up(){
  analogWrite(upDownMotorPIN, 2);
  delay(500);
  analogWrite(upDownMotorPIN, 0);
}
void down(){
  analogWrite(upDownMotorPIN, 252);
  delay(1200);
  analogWrite(upDownMotorPIN, 0);
}

void moveHorizontal(float dst) {
  int steps = dst / stepperCmPerStep;
  myStepper.step(steps);
}

void moveHorizontal(float dst, float inTime) {
  int steps = dst / stepperCmPerStep;
  int targetSpeed = round((float)abs(steps) / stepsPerRevolution / inTime);
  myStepper.setSpeed(targetSpeed);
  myStepper.step(steps);
  myStepper.setSpeed(STEPPER_MAX_SPEED);
}

void moveDiagonal(float dx, float dy) {
  float verticalDelay = getDelayVerticalMovement(abs(dx));
  moveVertical(dy, false);
  moveHorizontal(dx, verticalDelay);
  analogWrite(verticalMotorPIN, wypelnienieStop);
}

/*
  void moveDiagonalAngle(float dst, float angle) {
  float verticalDelay = getDelayVerticalMovement(abs(dx));
  moveVertical(dy, false);
  moveHorizontal(dx, verticalDelay);
  analogWrite(verticalMotorPIN, wypelnienieStop);
  }
*/

void setup()
{
  pinMode(verticalMotorPIN, OUTPUT);
  pinMode(upDownMotorPIN, OUTPUT);
  myStepper.setSpeed(15);
  analogWrite(verticalMotorPIN, wypelnienieStop);
  Serial.begin(9600);
  Serial.println("Executing initial delay");
  delay(5000);
  Serial.println("Arduino is ready");
}

void makeSquare(int side) {
  int dst = side;
  moveHorizontal(dst);
  moveVertical(dst);
  moveHorizontal(-dst);
  moveVertical(-dst);
}

void makeHexagon(int side) {
  float dst = side;
  float dstDiagonal = dst / sqrtf(2);
  moveVertical(-dst);
  moveDiagonal(dstDiagonal, -dstDiagonal);
  moveHorizontal(dst);
  moveDiagonal(dstDiagonal, dstDiagonal);
  moveVertical(dst);
  moveDiagonal(-dstDiagonal, dstDiagonal);
  moveHorizontal(-dst);
  moveDiagonal(-dstDiagonal, -dstDiagonal);
}

/*
void makeCircle(float r, int nTriangles) {
  float dist = sqrtf(r * r * (1 - 2 * cos(alpha)));
  float angle = 0;
  while (angle < 2 * 3.14) {
    float dx = r * cos(angle);
    float dy = r * sin(angle);
    moveDiagonal(dx, dy);
    angle += alpha;
  }
}
*/

char receivedChar;
boolean newData = false;
void recvOneChar() {
  if (Serial.available() > 0) {
    receivedChar = Serial.read();
    newData = true;
  }
}

boolean isDown = true;
void doMove() {
  /*
    0 - N
    1 - NE
    2 - E
    3 - SE
    4 - S
    5 - SW
    6 - W
    7 - NW
    8 - 90 stopni silnik od podnoszenia // clockwise
    9 - 90 stopni silnik do podnoszenia // counterclockwise
  */
  float dst = 1;
  float diagonalDst = dst / sqrtf(2);
  recvOneChar();
  if (newData) {
    newData = false;
    switch (receivedChar) {
      case '0':
        moveVertical(dst);
        break;
      case '7':
        moveDiagonal(diagonalDst, diagonalDst);
        break;
      case '6':
        moveHorizontal(dst);
        break;
      case '5':
        moveDiagonal(diagonalDst, -diagonalDst);
        break;
      case '4':
        moveVertical(-dst);
        break;
      case '3':
        moveDiagonal(-diagonalDst, -diagonalDst);
        break;
      case '2':
        moveHorizontal(-dst);
        break;
      case '1':
        moveDiagonal(-diagonalDst, diagonalDst);
        break;
      case '8':
        if(isDown){
          down();
          isDown = false;
        }else{
          up();
          isDown = true;
        }
        break;
      case '\n':
        break;
      default:
        Serial.println("F");
        return;
        break;
      
    }
    Serial.println("T");
  }
}

void loop()
{
  //makeSquare(1);
  //moveHorizontal(-2, 0.25f);
  //moveVertical(-5);
  //makeHexagon(3);
  //makeCircle(1, 3.14f / 3);
  
  doMove();
  
  
  /*
    analogWrite(verticalMotorPIN, 252);
    delay(10000);
    analogWrite(verticalMotorPIN, wypelnienie);
    //myStepper.step(-5 * stepsPerRevolution);
    delay(5000);
    analogWrite(verticalMotorPIN, wypelnienieStop);
    delay(10000);
  */
}
