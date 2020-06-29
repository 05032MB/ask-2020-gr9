//biblioteka udostępniające interfejs do obsługi silników krokowych
#include <Stepper.h>
//biblioteka pozwalająca na komunikację płytka -> komputer
#include <SoftwareSerial.h>

//numery odpowiednich pinów wykorzystywanych do kontroli silnika krokowego
#define IN1  7
#define IN2  6
#define IN3  5
#define IN4  10

#define UP_DOWN_DIODE_PIN 2  //pin do którego podłączona jest dioda wskazująca pozycję pisaka: GÓRA (świeci się), DÓŁ (nie świeci się)
#define FLASHING_DIODE_PIN 4  //pin do którego podłączona jest dioda migająca. Wskazuje ona na to, że kod programu wykonuje się bez żadnych zacięć.
#define FLASHING_DIODE_PERIOD 1000 //czas po jakim dioda migająca ma zmienić swój stan na przeciwny, w milisekundach

#define VERTICAL_MOTOR_PIN 3 // pin do którego podłączony jest silnik serwo odpowiadający za ruch w pionie względem kartki
#define UP_DOWN_MOTOR_PIN 9 // pin do którego podłączony jest silnik serwo odpowiadający za podnoszenie i opuszczanie pisaka


// liczba kroków silnika krokowego potrzebnych do wykonania pełnego obrotu
const int STEPS_PER_REVOLUTION = 2038;

// maksymalna prędkość silnika krokowego, w RPM
#define STEPPER_MAX_SPEED 15

// Instancja klasy Stepper odpowiadająca za sterowanie silnikiem krokowym.
Stepper myStepper = Stepper(STEPS_PER_REVOLUTION, IN1, IN3, IN2, IN4);



// odpowiednie wartości wypełnień potrzebne do różnych operacji
const int wypelnienieDown = 2;
const int wypelnienieStop = 0;
const int wypelnienieUp = 253;

// prędkości przemieszczania się pisaka po osiach za które odpowiadają poszczególne silniki. Prędkości te uzależnione są od wykorzystywanych śrub. 
const float serwoCmPM = 45.f * 0.1f * 5.f / 6.f; // / 5/6 dobrane eksperymentalnie. Miał przejechać 5 cm, przejechał 6 cm
const float stepperCmPM = 3.925e-4f;
const float stepperCmPerStep = 0.8f / STEPS_PER_REVOLUTION;

// zatrzymaj program na czas podany w minutach
void delayMinutes(float minutes) {
  delay(minutes * 60000);
}

// oblicz ile czasu zajmie przesunięcie pisaka o dany dystans (w cm) przez silnik serwo
float getDelayVerticalMovement(float dst) {
  float delayMinutes = dst / serwoCmPM;
  return delayMinutes;
}

// przesuń pisak w pionie o 'dst' centymentów. Znak dystansu wskazuje na kierunek ruchu: + w górę, - w dół
// argument doDelay mówi o tym, czy zatrzymać działanie programu do momentu skończenia pracy silnika
void moveVertical(float dst, boolean doDelay = true) {
  if (dst > 0)
    analogWrite(VERTICAL_MOTOR_PIN, wypelnienieUp);
  else if (dst < 0)
    analogWrite(VERTICAL_MOTOR_PIN, wypelnienieDown);
  if (doDelay) {
    delayMinutes(getDelayVerticalMovement(abs(dst)));
    analogWrite(VERTICAL_MOTOR_PIN, wypelnienieStop);
  }
}

// zapal diodę
void diodeON() {
  analogWrite(UP_DOWN_DIODE_PIN, 255);
}

// zgaś diodę
void diodeOFF() {
  analogWrite(UP_DOWN_DIODE_PIN, 0);
}

// podnieś pisak
void up() {
  analogWrite(UP_DOWN_MOTOR_PIN, 2);
  delay(1000);
  analogWrite(UP_DOWN_MOTOR_PIN, 0);
}

// opuść pisak
void down() {
  analogWrite(UP_DOWN_MOTOR_PIN, 252);
  delay(1000);
  analogWrite(UP_DOWN_MOTOR_PIN, 0);
}

// zmień stan diody na przeciwny
void diodeFlip() {
  static bool diode = false;
  if (diode) {
    diodeON();
  } else {
    diodeOFF();
  }
  diode = !diode;
}

// przesuń pisak w poziomie o 'dst' centymetrów. Znak dystansu mówi o kierunku: + w prawo, - w lewo
void moveHorizontal(float dst) {
  int steps = dst / stepperCmPerStep;
  myStepper.step(steps);
}

// to samo co moveHorizontal(float dst), tylko dobierana jest prędkość silnika aby skończył pracę w zadanycm czasie 'inTime' minut.
void moveHorizontal(float dst, float inTime) {
  int steps = dst / stepperCmPerStep;
  int targetSpeed = round((float)abs(steps) / STEPS_PER_REVOLUTION / inTime);
  myStepper.setSpeed(targetSpeed);
  myStepper.step(steps);
  myStepper.setSpeed(STEPPER_MAX_SPEED);
}

// przesuń pisak w dwóch osiach jednocześnie o wektor [dx, dy]
void moveDiagonal(float dx, float dy) {
  float verticalDelay = getDelayVerticalMovement(abs(dx));
  moveVertical(dy, false);
  moveHorizontal(dx, verticalDelay);
  analogWrite(VERTICAL_MOTOR_PIN, wypelnienieStop);
}

// funkcja inicjalizująca program. Wykonywana jest tylko raz przy starcie programu
void setup()
{
  pinMode(VERTICAL_MOTOR_PIN, OUTPUT);
  pinMode(UP_DOWN_MOTOR_PIN, OUTPUT);
  myStepper.setSpeed(15);
  analogWrite(VERTICAL_MOTOR_PIN, wypelnienieStop);
  Serial.begin(9600);
  Serial.println("Executing initial delay");
  delay(5000);
  Serial.println("Arduino is ready");
}

//zmienne oraz funkcja odpowiadające za odczytywanie przychodzących wiadomości
char receivedChar;
boolean newData = false;
void recvOneChar() {
  if (Serial.available() > 0) {
    receivedChar = Serial.read();
    newData = true;
  }
}

//funkcja wykonująca odpowiednie akcje na podstawie otrzymanych wiadomości przesyłanych przez UART z podłączonego komputera z oprogramowaniem sterującym rysowaniem
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
    8 - opuść / podnieś pisak
    9 - zaświeć diodą
  */
  float dst = 1;
  float diagonalDst = dst;
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
        if (isDown) {
          down();
          diodeOFF();
          isDown = false;
        } else {
          up();
          diodeON();
          isDown = true;
        }
        break;
      case '\n':
        break;
      case '9':
        diodeFlip();
        break;
      default:
        // otrzymano niepoprawny znak
        Serial.println("F");
        return;
        break;
    }
    // odczytywanie zakończone sukcesem
    Serial.println("T");
  }
}

// uaktualnienie stanu migającej diody
void updateFlashingDiode(){
  static unsigned long nextTime = FLASHING_DIODE_PERIOD;
  static bool on = false;
  unsigned long currentTime = millis();
  if(currentTime >= nextTime){
    nextTime = millis() + FLASHING_DIODE_PERIOD;
    if(on){
      analogWrite(FLASHING_DIODE_PIN, 0);
    }else{
      analogWrite(FLASHING_DIODE_PIN, 255);
    }
    on = !on;
  }
}

// główna pętla programu
void loop()
{
  updateFlashingDiode();
  doMove();
}