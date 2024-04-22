#define heater A2//高关低开，继电器是反的（x，别一直加热了

void setup()
{
    Serial.begin(115200);
    pinMode(heater, OUTPUT);
}
void loop()
{
    digitalWrite(heater, HIGH);
    Serial.println("write high but close");
    delay(5000);

    digitalWrite(heater, LOW);
    Serial.println("write low but open");
    delay(10000);
}