#include <Stepper.h>
#define STEPS 4076
Stepper stepper(STEPS, A0, A1, A2, A3);

enum ProcessState {
  INIT,
  FIRST_BEVERAGE_POURED,
  GLASS_MOVED,
  SECOND_BEVERAGE_POURED
};

const int RELAY_1 = 3;
const int RELAY_2 = 4;
const int FIRST_DRINK_SELECTED_BUTTON = 5;
const int SECOND_DRINK_SELECTED_BUTTON = 6;
const int BUZZER = 9;
const int FIRST_BEVERAGE_ULTRASONIC_TRIGGER = 12;
const int FIRST_BEVERAGE_ULTRASONIC_SENSOR = 10;
const int SECOND_BEVERAGE_ULTRASONIC_TRIGGER = 13;
const int SECOND_BEVERAGE_ULTRASONIC_SENSOR = 11;
const int MOTOR_TIME = 6000; // ms
const int BUZZER_FINISH_TIME = 500; // ms
const int BUZZER_ERROR_TIME = 3000; // ms
const int FULL_DRINK_TIME = 10000; // ms
float DRINK_1_TIME = 0.5; // ms
float DRINK_2_TIME = 0.5; // ms
int firstButtonPressed;
int secondButtonPressed;
ProcessState state;


void setup() {
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(FIRST_DRINK_SELECTED_BUTTON, INPUT);
  pinMode(SECOND_DRINK_SELECTED_BUTTON, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(FIRST_BEVERAGE_ULTRASONIC_TRIGGER, OUTPUT); //pin como salida
  pinMode(FIRST_BEVERAGE_ULTRASONIC_SENSOR, INPUT);  //pin como entrada
  digitalWrite(FIRST_BEVERAGE_ULTRASONIC_TRIGGER, LOW);
  pinMode(SECOND_BEVERAGE_ULTRASONIC_TRIGGER, OUTPUT); //pin como salida
  pinMode(SECOND_BEVERAGE_ULTRASONIC_SENSOR, INPUT);  //pin como entrada
  digitalWrite(SECOND_BEVERAGE_ULTRASONIC_TRIGGER, LOW);
  firstButtonPressed = LOW;
  secondButtonPressed = LOW;
  state = INIT;
  stepper.setSpeed(5);
   
  Serial.begin(9600);
}

void loop() {
  if (buttonsNotPressed()) {
    firstButtonPressed = digitalRead(FIRST_DRINK_SELECTED_BUTTON);
    secondButtonPressed = digitalRead(SECOND_DRINK_SELECTED_BUTTON);

    if (firstButtonPressed == HIGH) {
      Serial.println("se apreto el primer boton");
      DRINK_1_TIME = 0.7;
      DRINK_2_TIME = 0.3;
    } else if (secondButtonPressed == HIGH) {
      Serial.println("se apreto el segundo boton");
      DRINK_1_TIME = 0.4;
      DRINK_2_TIME = 0.6;
    }

  } else {
    if (state == INIT) {
      fillGlassUp(RELAY_1, DRINK_1_TIME, FIRST_BEVERAGE_POURED);
    } else if (state == FIRST_BEVERAGE_POURED) {
      moveGlass();
    } else if (state == GLASS_MOVED) {
      fillGlassUp(RELAY_2, DRINK_2_TIME, SECOND_BEVERAGE_POURED);
    } else if (state == SECOND_BEVERAGE_POURED) {
      Serial.println("Finalizado del proceso de llenado de vaso");
      turnBuzzerOn(BUZZER_FINISH_TIME);
      firstButtonPressed = LOW;
      secondButtonPressed = LOW;
      state = INIT;
    }
  }
}

int buttonsNotPressed() {
  return firstButtonPressed == LOW && secondButtonPressed == LOW;
}

void fillGlassUp(int relayPin, float delayPercentage, ProcessState nextState){
  if (isGlassInPlace()) {
    Serial.print("Llenando vaso con ");
    Serial.print(nextState);
    Serial.println("");
    digitalWrite(relayPin, HIGH);
    Serial.print("Tiempo de llenado: ");
    Serial.print(FULL_DRINK_TIME * delayPercentage);
    Serial.println("");
    delay(FULL_DRINK_TIME * delayPercentage);
    Serial.println("Fin llenado vaso");
    digitalWrite(relayPin, LOW);
    state = nextState;
    Serial.println("--------");
  } else {
    Serial.println("Vaso no apoyado");
    turnBuzzerOn(BUZZER_ERROR_TIME);
  }
}

int isGlassInPlace() {
  int currentTrigger = getCurrentTrigger();
  int currentSensor = getCurrentSensor();
  digitalWrite(currentTrigger, HIGH);
  delayMicroseconds(10);          //Enviamos un pulso de 10us
  digitalWrite(currentTrigger, LOW);
  long t = pulseIn(currentSensor, HIGH); //obtenemos el ancho del pulso
  long d = t/59;             //escalamos el tiempo a una distancia en cm
  Serial.print("Distancia medida: ");
  Serial.print(d);
  Serial.print(" cm en sensor");
  Serial.println(getCurrentSensor());
  if (d < 7) {
    Serial.println("Vaso apoyado");  
    return HIGH;
  }  
  return LOW;
}

int getCurrentTrigger() {
  if (state == INIT || state == FIRST_BEVERAGE_POURED){
    return FIRST_BEVERAGE_ULTRASONIC_TRIGGER;
  }
  if (state == GLASS_MOVED) {
    return SECOND_BEVERAGE_ULTRASONIC_TRIGGER;
  }
}

int getCurrentSensor() {
  if (state == INIT || state == FIRST_BEVERAGE_POURED){
    return FIRST_BEVERAGE_ULTRASONIC_SENSOR;
  }
  if (state == GLASS_MOVED) {
    return SECOND_BEVERAGE_ULTRASONIC_SENSOR;
  }
}

int moveGlass() {
  if (isGlassInPlace()) {
    Serial.println("Moviendo vaso");
    stepper.step(2048);
    Serial.println("Fin movida vaso");
    Serial.println("--------");
    state = GLASS_MOVED;
  } else {
    Serial.println("Vaso no apoyado");
    turnBuzzerOn(BUZZER_ERROR_TIME);
  }
}

boolean turnBuzzerOn(int delayTime) {
  digitalWrite(BUZZER, HIGH);
  delay(delayTime);
  digitalWrite(BUZZER, LOW);
  delay(delayTime);
}
