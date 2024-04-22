//MN Maker
//Laser Temp Gun
//10.6.19

#include <Wire.h>
#include <Adafruit_MLX90614.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

void setup() {
  Serial.begin(115200);
  Serial.println("Adafruit MLX90614 test"); 

  mlx.begin(); 
}

void loop() {
  
  Serial.print("Ambient = "); Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); Serial.print(mlx.readObjectTempC()); Serial.println("*C");

  Serial.println();
  delay(500);
}
