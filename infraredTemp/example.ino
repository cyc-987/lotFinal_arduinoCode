//MN Maker
//Laser Temp Gun
//10.6.19

#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

const int Laser_Pin=5;  //Laser Pin
int buttonState = 0; 
const int buttonPin = 2;     // the number of the pushbutton pin

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit MLX90614 test"); 
  
  pinMode(Laser_Pin,OUTPUT);
  pinMode(buttonPin, INPUT);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  

  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 35);
  display.println("Initializing Temp");
  display.display();
  delay(250);
  display.clearDisplay();

  mlx.begin(); 
  
}

void loop() {

  buttonState = digitalRead(buttonPin);
  Serial.println(buttonState);
  
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempF()); 
  Serial.print("*F\tObject = "); Serial.print(mlx.readObjectTempF()); Serial.println("*F");
  
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(Laser_Pin, HIGH);
    
    display.clearDisplay();
    display.setTextSize(2);  //Size 2 means each pixel is 12 width and 16 high
    display.setCursor(25, 10);
    display.print(mlx.readObjectTempC());
    display.setCursor(95, 10);
    display.print("C");
    display.setTextSize(2);
    display.setCursor(25, 36);
    display.print(mlx.readObjectTempF());
    display.setCursor(95, 36);
    display.print("F");
    display.display();
    
    
  } else {
    // turn LED off:
    digitalWrite(Laser_Pin, LOW);
  
    display.clearDisplay();
    display.setTextSize(2);  //Size 2 means each pixel is 12 width and 16 high
    display.setCursor(35, 10);
    display.print("-----");
    display.setCursor(105, 10);
    display.print("");
    display.setTextSize(2);
    display.setCursor(35, 36);
    display.print("-----");
    display.setCursor(105, 36);
    display.print("");
    display.display();
    
    
  }

  Serial.println();
  delay(500);
}
