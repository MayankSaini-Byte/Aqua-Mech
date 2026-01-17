// -------- MOTOR PINS (L298N) --------
#define ENA 5
#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define ENB 6

// -------- ULTRASONIC SENSOR --------
#define TRIG 2
#define ECHO 3

// -------- VARIABLES --------
char command;
int speed = 180;          // motor speed (0–255)
int obstacleLimit = 25;   // distance in cm
bool autoMode = false;
unsigned long lastCommand = 0;  // for feedback timing

// -------- SETUP --------
void setup()
{
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  Serial.begin(9600);   // Bluetooth baud rate
  stopBot();
  Serial.println("READY");  // Initial feedback
}

// -------- MAIN LOOP --------
void loop()
{
  if (Serial.available())
  {
    command = Serial.read();
    while(Serial.available()) Serial.read();  // Flush buffer to prevent queuing
    
    // Mode switching with feedback
    if (command == 'A') {
      autoMode = true;
      Serial.println("AUTO");
    }
    else if (command == 'M') {
      autoMode = false;
      Serial.println("MANUAL");
    }
    else if (!autoMode) {
      manualControl(command);
    }
    
    lastCommand = millis();
  }

  if (autoMode) {
    obstacleAvoid();
  }
  else if (millis() - lastCommand > 1000) {
    // Idle stop in manual after 1s no command
    stopBot();
  }
}

// -------- DISTANCE FUNCTION --------
long getDistance()
{
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duration = pulseIn(ECHO, HIGH, 30000);
  if (duration == 0) return 300;
  return duration * 0.034 / 2;
}

// -------- OBSTACLE MODE (Boat-optimized) --------
void obstacleAvoid()
{
  long distance = getDistance();

  if (distance <= obstacleLimit) {
    stopBot();
    delay(300);  // Longer stop for water momentum
    moveBackward();
    delay(500);  // Adjust for boat inertia
    turnLeft();  // Prefer left for boat races (counter-clockwise bias)
    delay(600);
    moveForward();
    Serial.println("OBSTACLE");  // Feedback
  }
  else {
    moveForward();
  }
}

// -------- MANUAL CONTROL --------
void manualControl(char cmd)
{
  switch (cmd) {
    case 'F': moveForward(); Serial.println("F"); break;
    case 'B': moveBackward(); Serial.println("B"); break;
    case 'L': turnLeft(); Serial.println("L"); break;
    case 'R': turnRight(); Serial.println("R"); break;
    case 'S': stopBot(); Serial.println("S"); break;
    case '1': speed = 120; Serial.println("SPD1"); break;
    case '2': speed = 160; Serial.println("SPD2"); break;
    case '3': speed = 200; Serial.println("SPD3"); break;
    case '4': speed = 255; Serial.println("SPD4"); break;
  }
}

// -------- MOTOR FUNCTIONS --------
void moveForward()
{
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward()
{
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnLeft()
{
  analogWrite(ENA, speed/2);  // Differential speed for smoother turns
  analogWrite(ENB, speed);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight()
{
  analogWrite(ENA, speed);
  analogWrite(ENB, speed/2);  // Differential for boat pivot
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopBot()
{
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);  // Better than digitalWrite LOW for PWM pins
}