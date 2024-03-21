// Arduino sketch to test the UtraSonic sensor
#include <NewPing.h>

void setup {
  Serial.begin(9600);
  NewPing us(12, 11, 200);


}

void loop {
  // Test the US sensor
  int distance = us.ping_cm(0);
  print("Distance: %d\n", distance);
  delay(1000);
}
