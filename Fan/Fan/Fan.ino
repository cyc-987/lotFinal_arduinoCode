#define fanPin 11

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(fanPin,OUTPUT); 
}

void loop() {
  // put your main code here, to run repeatedly:
  //delay(1000);
  digitalWrite(fanPin,HIGH);
  // delay(3000);
  // digitalWrite(fanPin,LOW);
}
