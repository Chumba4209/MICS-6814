
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


// OLED I2C Configuration

#define SDA_PIN 8
#define SCL_PIN 10

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(
    SCREEN_WIDTH,
    SCREEN_HEIGHT,
    &Wire,
    OLED_RESET
);


// MiCS-6814 Analog Pins

#define CO_PIN 0
#define NH3_PIN 1
#define NO2_PIN 3

// Voltage divider: 10k / 10k
// Original sensor voltage = ADC voltage * 2
const float DIVIDER_RATIO = 2.0;


// Configuration

bool oledReady = false;

const int NUM_SAMPLES = 20;
const unsigned long READ_INTERVAL = 2000;


// Read Averaged Analog Voltage

float readVoltage(uint8_t pin) {

    uint32_t total = 0;

    for (int i = 0; i < NUM_SAMPLES; i++) {

        total += analogReadMilliVolts(pin);
        delay(5);
    }

    // Average ADC voltage in millivolts
    float adcMilliVolts =
        (float)total / NUM_SAMPLES;

    // Calculate original sensor voltage
    float sensorMilliVolts =
        adcMilliVolts * DIVIDER_RATIO;

    return sensorMilliVolts;
}


// I2C Scanner

void scanI2C() {

    Serial.println("\nScanning I2C bus...");

    int devices = 0;

    for (uint8_t address = 1; address < 127; address++) {

        Wire.beginTransmission(address);

        if (Wire.endTransmission() == 0) {

            Serial.printf(
                "Device found at 0x%02X\n",
                address
            );

            devices++;
        }
    }

    Serial.printf("Total devices: %d\n", devices);
}


// OLED Startup

void showStartup() {

    if (!oledReady) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);

    display.setCursor(0, 0);
    display.println("MiCS-6814 SENSOR");
    display.println("--------------------");
    display.println("ESP32-C3 Mini");
    display.println("Initializing...");

    display.display();
}


// OLED Measurements

void showReadings(
    float co,
    float nh3,
    float no2
) {

    if (!oledReady) return;

    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);

    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("MiCS-6814 GAS SENSOR");

    display.drawLine(
        0, 12, 127, 12, SSD1306_WHITE
    );

    // CO channel
    display.setCursor(0, 19);
    display.print("CO : ");
    display.print(co, 0);
    display.println(" mV");

    // NH3 channel
    display.setCursor(0, 33);
    display.print("NH3: ");
    display.print(nh3, 0);
    display.println(" mV");

    // NO2 channel
    display.setCursor(0, 47);
    display.print("NO2: ");
    display.print(no2, 0);
    display.println(" mV");

    display.display();
}


// Setup

void setup() {

    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("==");
    Serial.println(" ESP32-C3 + MiCS-6814");
    Serial.println("==");

    // Initialize OLED I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000);
    Wire.setTimeOut(100);

    scanI2C();

    // Initialize ADC
    analogReadResolution(12);

    analogSetPinAttenuation(
        CO_PIN,
        ADC_11db
    );

    analogSetPinAttenuation(
        NH3_PIN,
        ADC_11db
    );

    analogSetPinAttenuation(
        NO2_PIN,
        ADC_11db
    );

    Serial.println("[ADC] Initialized");
    Serial.println("CO  -> GPIO0");
    Serial.println("NH3 -> GPIO1");
    Serial.println("NO2 -> GPIO3");

    // Initialize OLED
    oledReady = display.begin(
        SSD1306_SWITCHCAPVCC,
        OLED_ADDRESS,
        true,
        false
    );

    if (oledReady) {

        Serial.println("[OLED] Ready");
        showStartup();

    } else {

        Serial.println("[OLED] Not detected");
    }

    Serial.println("[MiCS-6814] Analog reading ready");

    delay(1000);
}


// Main Loop

void loop() {

    // Read sensor channel voltages
    float coVoltage = readVoltage(CO_PIN);
    float nh3Voltage = readVoltage(NH3_PIN);
    float no2Voltage = readVoltage(NO2_PIN);

    
    // Serial Monitor
    
    Serial.println();
    Serial.println("------ MiCS-6814 DATA ------");

    Serial.printf(
        "CO  : %.0f mV\n",
        coVoltage
    );

    Serial.printf(
        "NH3 : %.0f mV\n",
        nh3Voltage
    );

    Serial.printf(
        "NO2 : %.0f mV\n",
        no2Voltage
    );

    Serial.println("---------------------------");

    
    // OLED Display
    
    showReadings(
        coVoltage,
        nh3Voltage,
        no2Voltage
    );

    delay(READ_INTERVAL);
}
