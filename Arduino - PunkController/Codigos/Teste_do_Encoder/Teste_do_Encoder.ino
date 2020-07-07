#define encoderPinA  2
#define encoderPinB  3

volatile int direcao = 0;

void setup() {
  Serial.begin (9600);
  pinMode(encoderPinA, INPUT);
  pinMode(encoderPinB, INPUT);
  digitalWrite(encoderPinA, HIGH);       // turn on pullup resistor
  digitalWrite(encoderPinB, HIGH);       // turn on pullup resistor

  
  Serial.println("start");                // a personal quirk
  attachInterrupt(digitalPinToInterrupt(2), interrupcaoA, RISING);       //Borda de subida
  attachInterrupt(digitalPinToInterrupt(3), interrupcaoB, FALLING);       //Borda de descida
}

void loop() {
  //Serial.print(digitalRead(encoderPinA));Serial.println(digitalRead(encoderPinB));
  Serial.println(direcao);
  delay(50);
}


void interrupcaoA() {
  delay(1);
  if(digitalRead(encoderPinA) == 1 && digitalRead(encoderPinB) == 0)
    direcao++;
}

void interrupcaoB() {
  delay(1);
  if(digitalRead(encoderPinA) == 1 && digitalRead(encoderPinB) == 0)
    direcao--;
}
