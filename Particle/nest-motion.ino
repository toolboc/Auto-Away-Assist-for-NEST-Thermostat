/*
*********************
Nest Motion Detection
*********************
A tunable program for determining motion events using a PIR sensor, with an emphasis on reducing false-positives and minimizing energy consumption.

When motion is detected, an event is published to Particle.io.
This event is then forwarded to an Azure IoT Hub which calls a Serverless function informing Nest to set Away Status to "Home"

Instruction for end-to-end configuration are avaiable @ 
*/


#include "CircularBuffer.h"

#define PIRPIN D0 //The Pin attached to the PIR Sensor

const int CalibrationTimeInSeconds = 30; //Seconds to await for calibration of the PIR sensor
const int SampleWindowSize = 10; //Ex: SampleWindowSize is the amount of samples to keep track of for evaluating the occurence of a motion event
const int PostiveSamplesToTriggerMotion = 10; //Ex: A value of x will require that at least x samples produced within the sampleWindow are postive to trigger a motion event 
const int SleepIntervalInSeconds = 600;  //The amount of time to go into deep sleep mode afer motion is reported, 600 seconds equates to a max of 10 alerts per hour

CircularBuffer<bool, SampleWindowSize> sampleWindow;

char output[50];

//Per PIR spec, allow 30s to calibrate (warm up) the sensor
void CalibrateSensor()
{
    Serial.print("Calibrating Sensor... ");
    for(int i = 0; i < CalibrationTimeInSeconds; i++){
        delay(1000);
    }
    Serial.println("PIR Sensor Calibrated");
}

void setup() {

    Serial.begin(9600);

    Serial.println("***************************************");
    Serial.println("    Nest Motion Detection Started ");
    Serial.println("***************************************");

    pinMode(PIRPIN, INPUT);
    CalibrateSensor();


}

void loop() {

    sampleWindow.push(SamplePIR());

    if(CheckSampleWindowForMotion())
    {
        Serial.print("Publishing motion event... ");
        //Motion accurately detected, time to inform Nest that we are we are home
        Particle.publish("motion",  "true", PRIVATE); //Trigger the integration
        delay(1000); //Extra sleep to ensure message delivery
        Serial.println("Motion event published");
        Serial.println("Going to sleep now...");
        System.sleep(SLEEP_MODE_DEEP, SleepIntervalInSeconds); //Go into deep sleep low-power mode for SleepIntervalInSeconds seconds

        CalibrateSensor();//Recalibrate Sesnor on awaken
    }
    
}

//Takes ten readings per second, returns true if a postive reading is encountered
bool SamplePIR() {
    
    Serial.print("Sampling PIR... ");
    
    int val = 0;
    for(int i = 0; i < 10; i += 1)
    {
        if(val == LOW)
            val = digitalRead(PIRPIN);
        
        delay(100);
    }
    
    if(val)
     {
         Serial.println(" Motion Detected in sample!");
         return true;
     }
    else
    {
        Serial.println(" No Motion Detected in sample");
        return false;
    }
}

//Loops through the sampleWindow, returns true if enough positive samples are found
bool CheckSampleWindowForMotion()
{
    Serial.print("Checking Sample Window... ");
    int positiveSamples = 0;
    
    for(int i = 0; i < SampleWindowSize ; i++){
        if(sampleWindow.pop() == true)
            positiveSamples++;
    }
    
    Serial.print(positiveSamples);
    Serial.println(" positive samples were found in sample window");
    
    if(positiveSamples >= PostiveSamplesToTriggerMotion)
        return true;
    else
        return false;
}
