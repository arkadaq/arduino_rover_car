

#define MOTOR_A_SPEED_PIN 10   // Motor A: speed pin
#define MOTOR_B_SPEED_PIN 11   // Motor B: speed pin
#define MOTOR_A_DIR_PIN   12   // Motor A: direction pin
#define MOTOR_B_DIR_PIN   13   // Motor B: direction pin

#define MOTOR_LEFT_SPEED_PIN  MOTOR_A_SPEED_PIN
#define MOTOR_LEFT_DIR_PIN    MOTOR_A_DIR_PIN
#define MOTOR_RIGHT_SPEED_PIN MOTOR_B_SPEED_PIN
#define MOTOR_RIGHT_DIR_PIN   MOTOR_B_DIR_PIN

#define BUZZER_PIN 4

#define STICK_X_PIN  A4
#define STICK_Y_PIN  A5
#define STICK_SW_PIN A2

#define SECOND 1000

#define pinModeInput(pin)  pinMode(pin, INPUT)
#define pinModeOutput(pin) pinMode(pin, OUTPUT)
#define UndefinedPin -1

class Motor {
public:  
  void attach(int speed_pin, int direction_pin) {
    pinModeOutput(speed_pin);
    pinModeOutput(direction_pin);
    this->speed_pin = speed_pin;
    this->direction_pin = direction_pin;
  }
  
  void drive(int speed) {
    #define BACKWARDS LOW
    #define FORWARDS  HIGH    
    digitalWrite(direction_pin, speed < 0 ? BACKWARDS : FORWARDS);

    if (speed < 0) {
      speed = map(-speed, 0, 128, 0, 255);
    }    
    else {
      speed = map(speed, 0, 127, 0, 255);
    }
    
    analogWrite(speed_pin, speed);
  }
protected:
  int speed_pin = UndefinedPin, direction_pin = UndefinedPin;
};

struct XY {
  int x; // left / right
  int y; // speed backwards / forwards
  int sw; // switch 
};

struct LeftRight {
  int left, right;
};

void log(int x, int y, int left, int right) {
  Serial.print("x = ");
  Serial.print(x);
  Serial.print(", y = ");
  Serial.print(y);
  Serial.print(", left = ");
  Serial.print(left);
  Serial.print(", right = ");
  Serial.print(right);
  Serial.println();
}

int quant_step16_shifted(int x) {
  return 16 + (((x - 8) >> 4) << 4);
}

LeftRight from_XY(const XY &xy) {
  LeftRight lr;
  
  #define STICK_MID 128

  int direction = quant_step16_shifted(xy.x - STICK_MID);
  int speed     = quant_step16_shifted(xy.y - STICK_MID);

  // left:  left motor - forwards, right motor - backwards
  // right: left motor - backwards, right motor - forwards

  if (direction < 0) { // Left
    lr.left  = speed;
    lr.right = -speed;
  }
  else if (direction == 0) { // Straight
    lr.left = lr.right = speed;    
  }
  else { // Right
    lr.left  = -speed;
    lr.right = speed;
  }

  log(xy.x, xy.y, lr.left, lr.right);

  return lr;
}

class StickPadXY {
public:
  void attach(int x_pin, int y_pin, int sw_pin) {
    pinModeInput(x_pin);
    pinModeInput(y_pin);
    pinModeInput(sw_pin);
    this->x_pin = x_pin;
    this->y_pin = y_pin;
    this->sw_pin = sw_pin;
  }

  static int analogRead8bit(int pin) {
    return analogRead(pin) >> 2;
  }
  
  XY readStick() {
    return { analogRead8bit(x_pin), analogRead8bit(y_pin), 0 };
  }
  
  int x_pin = UndefinedPin, y_pin = UndefinedPin, sw_pin = UndefinedPin;  
};

Motor left_motor, right_motor;
StickPadXY stick_pad;

void startup_checks() {
  //tone(BUZZER_PIN, 1000);  

  // go forwards
  drive(127, 127);
  delay(2 * SECOND);

  drive(0, 0);// brake
  delay(1 * SECOND);
  
  // go backwards
  drive(-128, -128);
  delay(2 * SECOND);

  drive(0, 0);// brake
  delay(1 * SECOND);
    
  // rotate clockwise
  drive(127, -128);
  delay(2 * SECOND);

  drive(0, 0);// brake
  delay(1 * SECOND);
  
  // rotate counterclockwise
  drive(-128, 127);
  delay(2 * SECOND);  

  //noTone(BUZZER_PIN);
}

void setup() {
  // put your setup code here, to run once:

  left_motor.attach(MOTOR_LEFT_SPEED_PIN, MOTOR_LEFT_DIR_PIN);
  right_motor.attach(MOTOR_RIGHT_SPEED_PIN, MOTOR_RIGHT_DIR_PIN);
  stick_pad.attach(STICK_X_PIN, STICK_Y_PIN, STICK_SW_PIN);

  pinModeOutput(BUZZER_PIN);

  Serial.begin(9600);

  //startup_checks();
}

void loop() {
  // put your main code here, to run repeatedly:
  LeftRight lr = from_XY(stick_pad.readStick());
  drive(lr.left, lr.right);
}

void drive_debug(int left, int right) {
  Serial.print("left = ");
  Serial.print(left);
  Serial.print(", right = ");
  Serial.print(right);
  Serial.println();
}

void drive(int left, int right) {
  return drive_debug(left, right);

  left_motor.drive(left);
  right_motor.drive(right);
}
