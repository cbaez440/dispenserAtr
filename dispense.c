enum ProcessState {
  INIT,
  FIRST_BEVERAGE_POURED,
  GLASS_MOVED,
  SECOND_BEVERAGE_POURED
};

const int RELAY_1 = 3;
const int RELAY_2 = 4;
const int DRINK_SELECTED_BUTTON = 5;
const int STEPPER_MOTOR = 7;
const int BUZZER = 9;
const int FIRST_BEVERAGE_ULTRASONIC_TRIGGER = 12;
const int FIRST_BEVERAGE_ULTRASONIC_SENSOR = 10;
const int SECOND_BEVERAGE_ULTRASONIC_TRIGGER = 13;
const int SECOND_BEVERAGE_ULTRASONIC_SENSOR = 11;
const int DRINK_1_TIME = 5000; // ms
const int DRINK_2_TIME = 5000; // ms
const int MOTOR_TIME = 6000; // ms
const int BUZZER_FINISH_TIME = 500; // ms
const int BUZZER_ERROR_TIME = 3000; // ms
int isStartButtonPressed;
ProcessState state;


void setup() {
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(DRINK_SELECTED_BUTTON, INPUT);
  pinMode(STEPPER_MOTOR, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(FIRST_BEVERAGE_ULTRASONIC_TRIGGER, OUTPUT); //pin como salida
  pinMode(FIRST_BEVERAGE_ULTRASONIC_SENSOR, INPUT);  //pin como entrada
  digitalWrite(FIRST_BEVERAGE_ULTRASONIC_TRIGGER, LOW);
  pinMode(SECOND_BEVERAGE_ULTRASONIC_TRIGGER, OUTPUT); //pin como salida
  pinMode(SECOND_BEVERAGE_ULTRASONIC_SENSOR, INPUT);  //pin como entrada
  digitalWrite(SECOND_BEVERAGE_ULTRASONIC_TRIGGER, LOW);
  isStartButtonPressed = LOW;
  state = INIT;
   
  Serial.begin(9600);
}

void loop() {
  if (isStartButtonPressed == LOW) {
    // Posible lugar de lectura de selección de bebida, modificando las variables DRINK_1_TIME y DRINK_2_TIME
    isStartButtonPressed = digitalRead(DRINK_SELECTED_BUTTON);
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
      isStartButtonPressed = 0;
      state = INIT;
    }
  }
}

void fillGlassUp(int relayPin, int delayTime, ProcessState nextState){
  if (isGlassInPlace()) {
    Serial.print("Llenando vaso con ");
    Serial.print(nextState);
    Serial.println("");
    digitalWrite(relayPin, HIGH);
    delay(delayTime);
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
    digitalWrite(STEPPER_MOTOR, HIGH);
    delay(MOTOR_TIME);
    Serial.println("Fin movida vaso");
    Serial.println("--------");
    digitalWrite(STEPPER_MOTOR, LOW);
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

