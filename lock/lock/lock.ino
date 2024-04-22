#define lockInput A4
#define lockOutput A5

void setup() {
  Serial.begin(115200);
  pinMode(lockOutput, INPUT_PULLUP);         //Set Pin6 as input
  pinMode(lockInput, OUTPUT);
}
void loop() {
  digitalWrite(lockInput, HIGH);
  Serial.println("write high");
  Serial.println(digitalRead(lockOutput));
  delay(2000);

  digitalWrite(lockInput, LOW);
  Serial.println("write low");
  Serial.println(digitalRead(lockOutput));
  delay(2000);

}