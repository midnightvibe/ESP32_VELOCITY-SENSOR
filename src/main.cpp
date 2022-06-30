// Author: Marcel Kaminski
// Update:

// Imported the Code from John Wasser...
// Next Step add the momentary pole or the Euler's velocity formula in order to get the velocity from
// the bicycle centre / axis then display it on an lcd. Last step make it visible on a cellphone (bluetooth)

#include <Arduino.h>

// Configuration constants:
const byte RevSensePin = 2;
const float WheelRadiusInMeters = 0.33;
const unsigned long DisplayIntervalMillis = 1000;  // Update once per second
const unsigned long MaxRevTimeMicros = 2000000UL; // >2 seconds per revolution counts as 0 RPM

// Variables used in the ISR and in the main code must be 'volatile'
volatile unsigned long RevSenseTimeMicros = 0;  //  Time that the rising edge was sensed
volatile unsigned long RevTimeMicros = 0;  // Microseconds between consecutive pulses

// Useful constants:
const unsigned long SixtySecondsInMicros = 60000000UL;
const float WheelCircumferenceInMeters = TWO_PI * WheelRadiusInMeters;

void setup()
{
    Serial.begin(9600);
    pinMode(RevSensePin, INPUT);
    attachInterrupt(digitalPinToInterrupt(RevSensePin), RevSenseISR, RISING);
}

void RevSenseISR()
{
    static unsigned long revSensePreviousMicros = 0;  // 'static' to retain value between calls

    RevSenseTimeMicros = micros();
    RevTimeMicros = RevSenseTimeMicros - revSensePreviousMicros; // Time for last revolution
}

void loop()
{
    static unsigned previousRPM;

    // Only update the display once per DisplayIntervalMillis
    unsigned long currentMillis = millis();
    static unsigned long previousMillis = 0;
    if (currentMillis - previousMillis >= DisplayIntervalMillis)
    {
        previousMillis += DisplayIntervalMillis;

        // With interrupts disabled, make local copies of volatile variables
        // This is so the ISR can't change them while we read them
        noInterrupts();
        unsigned long revSenseTimeMicros = RevSenseTimeMicros;  //  Time that the last rising edge was sensed
        unsigned long revTimeMicros = RevTimeMicros;  // Microseconds between consecutive pulses
        interrupts();

        // Calculate RPM
        unsigned newRPM;
        if (micros() - revSenseTimeMicros > MaxRevTimeMicros)
            newRPM = 0;   // Going so slow we're essentially stopped
        else
            newRPM = SixtySecondsInMicros / revTimeMicros;

        // No need to update the display unless the RPM value has changed
        if (newRPM != previousRPM)
        {
            previousRPM = newRPM;
            displayRPM(newRPM);
        }
    }
}

void displayRPM(unsigned RPM)
{
    float metersPerMinute = RPM * WheelCircumferenceInMeters;

    Serial.print("RPM = "); //print the word "RPM".
    Serial.print(RPM); // print the rpm value.
    Serial.print("\t\t Linear Speed = ");
    Serial.print(metersPerMinute / 60.0); //print the linear velocity value.
    Serial.println(" m/s");
}