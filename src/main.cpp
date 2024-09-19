
#include <Arduino.h>

// Connect to the two encoder outputs!
#define ENCODER_A   14
#define ENCODER_B   27

// These let us convert ticks-to-RPM
#define GEARING     20.4  // To be tested
#define ENCODERMULT 12.0  // To be tested

#define a1  26
#define a2  25

volatile uint32_t lastA = 0;
volatile bool motordir = 0;
volatile uint32_t pulseCount = 0;
float RPM = 0;

// PWM settings
const int PWMFreq = 5000;    // Frequency of the PWM signal in Hz
const int PWMResolution = 8; // 8-bit resolution, so duty cycle ranges from 0 to 255
const int PWMChannel1 = 0;
const int PWMChannel2 = 1;

void IRAM_ATTR interruptA() {
    motordir = digitalRead(ENCODER_B);

    uint32_t currA = micros();
    uint32_t diffA = currA - lastA;
    if (lastA > currA) {
        diffA = (0xFFFFFFFF - lastA) + currA;
    }

    pulseCount++;

    lastA = currA;
}

void setup() {
    Serial.begin(115200);           // set up Serial library at 9600 bps
    Serial.println("MMMMotor party!");

    pinMode(ENCODER_B, INPUT_PULLUP);
    pinMode(ENCODER_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(ENCODER_A), interruptA, RISING);

    // Configure PWM for a1 and a2
    ledcSetup(PWMChannel1, PWMFreq, PWMResolution); // Set up PWM for channel 1
    ledcSetup(PWMChannel2, PWMFreq, PWMResolution); // Set up PWM for channel 2

    // Attach PWM channels to GPIO pins
    ledcAttachPin(a1, PWMChannel1);
    ledcAttachPin(a2, PWMChannel2);

    // Start with motor stopped (duty cycle = 0)
    ledcWrite(PWMChannel1, 0); // Set initial duty cycle to 0 (stopped)
    ledcWrite(PWMChannel2, 0); // Set initial duty cycle to 0 (stopped)

    delay(100);
}

void controlMotorSpeed(int dutyCycle, bool dir) {
    // Ensure dutyCycle is within the range of 0 to 255
    dutyCycle = constrain(dutyCycle, 0, 255);

    // Apply PWM to the appropriate pins for motor control
    if (dir) {
        // If motor direction is CCW
        ledcWrite(PWMChannel1, 0);      // a1 LOW
        ledcWrite(PWMChannel2, dutyCycle); // a2 PWM control
    } else {
        // If motor direction is CW
        ledcWrite(PWMChannel1, dutyCycle); // a1 PWM control
        ledcWrite(PWMChannel2, 0);      // a2 LOW
    }
}

void printRPM() {

    Serial.print("Direction: ");
    if (motordir) {
        Serial.print("CCW @ ");
    } else {
        Serial.print("CW @ ");
    }
    Serial.print((int)RPM); Serial.println(" RPM");
}

void calculateRPM() {
    static uint32_t lastPulseCount = 0;
    static uint32_t lastCalcTime = 0;

    uint32_t currentTime = millis();
    uint32_t deltaTime = currentTime - lastCalcTime;

    if (deltaTime >= 1000) {  // Calculate every 1 second
        uint32_t deltaPulses = pulseCount - lastPulseCount;

        // Calculate RPM
        float revs = deltaPulses;
        revs /= GEARING;                // account for gear ratio
        revs /= ENCODERMULT;            // account for multiple ticks per rotation
        revs *= (60000.0 / deltaTime);  // convert to RPM (revolutions per minute)

        RPM = revs;

        // Update for next calculation
        lastPulseCount = pulseCount;
        lastCalcTime = currentTime;
    }
}

void loop() {

    for (int i = 0; i < 255; i++) {
        controlMotorSpeed(i, true);
        calculateRPM();
        printRPM();
        delay(10);
    }

    for (int i = 0; i < 255; i++) {
        controlMotorSpeed(i, false);
        calculateRPM();
        printRPM();
        delay(10);
    }

}