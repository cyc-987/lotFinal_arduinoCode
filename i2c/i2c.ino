#include <Wire.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup()
{
    Serial.begin(115200);
    Serial.println("Adafruit MLX90614 test");

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
    display.setCursor(0, 35);
    display.println("Initializing Temp");
    display.display();
    delay(250);
    display.clearDisplay();

    mlx.begin();
}

void loop()
{

    Serial.print("Ambient = ");
    Serial.print(mlx.readAmbientTempC());
    Serial.print("*C\tObject = ");
    Serial.print(mlx.readObjectTempC());
    Serial.println("*C");

    display.clearDisplay();
    display.setTextSize(2); // Size 2 means each pixel is 12 width and 16 high
    display.setCursor(0,0);
    display.print(mlx.readObjectTempC());
    //display.setCursor(95, 10);
    display.print("C");
    display.display();

    Serial.println();
    delay(500);
}
