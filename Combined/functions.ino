//#include "Combined.ino"
void dataInit()
{
    temperature = 0;
    humidity = 0;
    lockStatus = 0;
    hasTakeout = 0;
    heaterStatus = 0;
    fanStatus = 0;
    timeStart = 0;
    targetTemperature = 0;
    targetHumidity = 0;
    voiceOutput = 0;
    lightswitch = 0;
    locallockStatus = 0;
}

// #TODO: add voice control
void voiceInit()
{
    pinMode(openlockvoicePin, OUTPUT);
    analogWrite(openlockvoicePin, 255);
}
void voiceCtrl(int voice)
{
    if(voice == 1){
        //语音播报
        analogWrite(openlockvoicePin, 0);
        delay(500);
        analogWrite(openlockvoicePin, 255);
    }
}


void lightInit(){
  pinMode(7,OUTPUT);  //RGB引脚
  pinMode(8,OUTPUT); 
  pinMode(9,OUTPUT); 
}
void lightCtrl(int light){
  if(light == 1){
    analogWrite(7,255);
    analogWrite(8,255);
    analogWrite(9,255);
  }else{
    analogWrite(7,0);
    analogWrite(8,0);
    analogWrite(9,0);
  }
}

//fan functions
void fanInit()
{
    pinMode(fanPin,OUTPUT); 
    digitalWrite(fanPin,LOW);
}
void fanCtrl(int status){
    if(status == 1){
        digitalWrite(fanPin,HIGH);
        fanStatus = 1;
    }else{
        digitalWrite(fanPin,LOW);
        fanStatus = 0;
    }
}

//dht11 functions
void getDHT11Data()
{
    int chk;
    //Serial.print("DHT11, \t");
    chk = DHT.read(DHT11_PIN);    // READ DATA
    switch (chk){
    case DHTLIB_OK:
                //Serial.print("OK,\t");
                break;
    case DHTLIB_ERROR_CHECKSUM:
                Serial.print("Checksum error,\t");
                break;
    case DHTLIB_ERROR_TIMEOUT:
                Serial.print("Time out error,\t");
                break;
    default:
                Serial.print("Unknown error,\t");
                break;
    }
    // DISPLAT DATA
    //Serial.print("Humidity: ");
    //Serial.println(DHT.humidity,1);
    //Serial.print(",\t");
    //Serial.println(DHT.temperature,1);

    //save data
    //temperature = DHT.temperature;
    humidity = DHT.humidity;

    //delay(2000);
}

//oled functions
void oledInit()
{
    //u8g2.begin();
    //Wire.begin();
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    { // Address 0x3D for 128x64
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ;
    }
    display.clearDisplay();
    display.setRotation(4);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Initializing");
    display.display();
    delay(500);
    display.clearDisplay();
}
/*
void oledDisplay()
{
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.drawStr(0,24,"Hello World!");
    } while ( u8g2.nextPage() );
}
*/
//oled 显示温湿度
void oledDisplay_th(){
    /*
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_6x10_tr); // 稍大的字体
        u8g2.setCursor(0, 12);
        u8g2.print("Temp: ");
        u8g2.print(temperature, 1);
        u8g2.setCursor(0, 24);
        u8g2.print("Hum: ");
        u8g2.print(humidity, 1);
        u8g2.setCursor(0, 36);
        u8g2.print("Deliv: ");
        u8g2.print(hasTakeout? "Yes" : "No");
        u8g2.setCursor(0, 48);
        u8g2.print("Lock: ");
        u8g2.print(lockStatus ? "No" : "Yes");
    } while ( u8g2.nextPage() );
    */
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0,0);
    display.print("Temp: ");
    display.print(temperature, 1);
    display.setCursor(0, 15);
    display.print("Hum: ");
    display.print(humidity, 1);
    display.setCursor(0, 30);
    display.print("Deliv: ");
    display.print(hasTakeout ? "Yes" : "No");
    display.setCursor(0, 45);
    display.print("Lock: ");
    display.print(locallockStatus ? "No" : "Yes");
    display.display();
}
//lock functions
void lockInit()
{
    pinMode(lockOutput, INPUT_PULLUP); 
    pinMode(lockInput, OUTPUT);
    digitalWrite(lockInput, LOW);//set to close
}
void lockCtrl(int status)
{
    //0 is lock, 1 is unlock
    if(status == 1){
        digitalWrite(lockInput, HIGH);
        delay(2000);
        digitalWrite(lockInput, LOW);
        lockStatus=0;
        voiceCtrl(1);
    }else{
        digitalWrite(lockInput, LOW);
    }
}
void lockCheck()
{
    locallockStatus = digitalRead(lockOutput);
}

//heater functions
void heaterInit()
{
    pinMode(heater, OUTPUT);
}
void heaterCtrl(int status)
{
    //0 is off, 1 is on
    if(status == 1){
        digitalWrite(heater, LOW);
        heaterStatus = 1;
    }else{
        digitalWrite(heater, HIGH);
        heaterStatus = 0;
    }
}

//infrared temperature functions
void itemperatureInit()
{
    mlx.begin();
}
void itemperatureRead()
{

    temperature = mlx.readObjectTempC();


}

//data control functions
void getData()
{

    //humidity
    getDHT11Data();
    //temperature
    itemperatureRead();
    //lock
    lockCheck();
}

void updateData()
{
    //fan
    if (humidity < targetHumidity - 3) {

        fanStatus = 0;
    } else if (humidity > targetHumidity + 3) {

        fanStatus = 1;
    }
    fanCtrl(fanStatus);

    //heater
    if (temperature < targetTemperature - 3) {

        heaterStatus = 1;
    } else if (temperature > targetTemperature + 3) {

        heaterStatus = 0;
    }
    heaterCtrl(heaterStatus);

    //lock
    if(lockStatus == 1){
        lockCtrl(1);
    }else{
        lockCtrl(0);
    }

    //oled
    oledDisplay_th();
    //voice
    //light
    if(lightswitch == 1 && locallockStatus == 1){
        lightCtrl(1);
    }else{
        lightCtrl(0);
        lightswitch = 0;
    }
}