#include "Upload.ino"
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
    }else{
        digitalWrite(fanPin,LOW);
    }
}

//dht11 functions
void getDHT11Data()
{
    int chk;
    Serial.print("DHT11, \t");
    chk = DHT.read(DHT11_PIN);    // READ DATA
    switch (chk){
    case DHTLIB_OK:
                Serial.print("OK,\t");
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
    Serial.print(DHT.humidity,1);
    Serial.print(",\t");
    Serial.println(DHT.temperature,1);

    //save data
    temperature = DHT.temperature;
    humidity = DHT.humidity;

    //delay(2000);
}

//oled functions
void oledInit()
{
    u8g2.begin();
}
void oledDisplay()
{
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB14_tr);
        u8g2.drawStr(0,24,"Hello World!");
    } while ( u8g2.nextPage() );
}

//lock functions
void lockInit()
{

}
void lockCtrl(int status)
{

}
void lockCheck()
{

}

//heater functions

//data control functions
void getData()
{
    getDHT11Data();
    lockCheck();
}

void updateData()
{
    //fan
    if(humidity < targetHumidity-3 || humidity > targetHumidity+3){
        fanStatus = 1;
        fanCtrl(fanStatus);
    }else{
        fanStatus = 0;
        fanCtrl(fanStatus);
    }

    //heater
    if(temperature < targetTemperature-3 || temperature > targetTemperature+3){
        heaterStatus = 1;
    }else{
        heaterStatus = 0;
    }

    //lock

    //oled

    //voice
}