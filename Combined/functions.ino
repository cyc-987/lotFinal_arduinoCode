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
    //temperature = DHT.temperature;
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
    }else{
        digitalWrite(lockInput, LOW);
    }
}
void lockCheck()
{
    lockStatus = digitalRead(lockOutput);
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
        heaterCtrl(heaterStatus);
    }else{
        heaterStatus = 0;
        heaterCtrl(heaterStatus);
    }

    //lock
    if(lockStatus == 1){
        lockCtrl(1);
    }else{
        lockCtrl(0);
    }

    //oled

    //voice
}