#include <M5StickCPlus.h>

void setup()
{
    M5.begin();

    Serial1.begin(9600, SERIAL_8N1, 33, 32); // Grove
}

void loop()
{
    // Serial(PC) to Serial1(Grove)
    if (Serial.available())
    {
        int inByte = Serial.read();
        Serial1.write(inByte);
    }

    // Serial1(Grove) to Serial(PC)
    if (Serial1.available())
    {
        int inByte = Serial1.read();
        Serial.write(inByte);
    }
}
