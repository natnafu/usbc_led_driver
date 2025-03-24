#include <Arduino.h>
#include <SparkFun_STUSB4500.h>
#include <Wire.h>

// STUSB4500 Definitions
#define PIN_STUSB4500_SDA 2
#define PIN_STUSB4500_SCL 3
#define PIN_STUSB4500_RESET 0
#define STUSB4500_ADDR  0x28
#define SW_GPIO_ADDR 0x2D

// LED Strip Pin Definitions
#define PIN_BLUE 7
#define PIN_GREEN 5
#define PIN_RED 6
#define PIN_WW 4
#define PIN_CW 10

// Switch pin high = 12V
#define PIN_SW_12V 1

// PD settings
#define LED_VOLTAGE 12 // voltage of LEDs set by switch if not 5V
#define CURR_LIMIT_A 3.0
#define MAX_LED_VOLTAGE 15
// safety check
#if LED_VOLTAGE > MAX_LED_VOLTAGE
  #error "LED_VOLTAGE must be less than MAX_LED_VOLTAGE"
#endif

// LED PWM things
#define PWM_FREQ 5000
#define PWM_RES 8
#define PWM_CH_BLUE 0
#define PWM_CH_GREEN 1
#define PWM_CH_RED 2
#define PWM_CH_WW 3
#define PWM_CH_CW 4

STUSB4500 usb;

void setUSBGPIO(uint8_t value) {
  Wire.beginTransmission(STUSB4500_ADDR);
  Wire.write(SW_GPIO_ADDR);
  Wire.write(value ? 0x01 : 0x00);
  Wire.endTransmission();
}

void usb_pd_config() {
  Serial.println("Configuring STUSB4500...");
  usb.read(); // get current settings

  // determine voltage based off switch
  float voltage = 5.0;
  if (digitalRead(PIN_SW_12V)) voltage = LED_VOLTAGE;
  Serial.printf("Switch set to %.1fV\n", voltage);

  // if 12V, turn on USB GPIO LED
  if (voltage == LED_VOLTAGE) {
    setUSBGPIO(1);
  } else {
    setUSBGPIO(0);
  }

  // check if voltage and current need to be updated
  bool update_pd = false;
  if (usb.getVoltage(2) != voltage) {
    Serial.println("Voltage needs to be updated.");
    update_pd = true;
  } else if (usb.getCurrent(2) != CURR_LIMIT_A) {
    Serial.println("Current needs to be updated.");
    update_pd = true;
  }

  if (!update_pd) {
    Serial.println("STUSB4500 already configured.");
    return;
  }

  // configure PD settings
  usb.setPdoNumber(2);
  // voltage settings
  Serial.printf("Setting PDO2 voltage to %.1fV\n", voltage);
  usb.setVoltage(2, voltage);
  usb.setUpperVoltageLimit(2,10);
  usb.setLowerVoltageLimit(2,10);
  // current settings
  Serial.printf("Setting PDO2 current to %.1fA\n", CURR_LIMIT_A);
  usb.setCurrent(2,CURR_LIMIT_A);
  // other settings
  usb.setFlexCurrent(1.0);
  usb.setExternalPower(true);
  usb.setConfigOkGpio(2);
  usb.setGpioCtrl(0);
  usb.setPowerAbove5vOnly(false);
  usb.setReqSrcCurrent(true);

  usb.write();
  usb.softReset();
  Serial.println("STUSB4500 configured.");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  // setup pins
  pinMode(PIN_BLUE, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_WW, OUTPUT);
  pinMode(PIN_CW, OUTPUT);
  pinMode(PIN_STUSB4500_RESET, OUTPUT);
  pinMode(PIN_SW_12V, INPUT_PULLUP);

  // setup USB PD IC
  digitalWrite(PIN_STUSB4500_RESET, LOW); // make sure ST is enabled
  Wire.begin(PIN_STUSB4500_SDA, PIN_STUSB4500_SCL);
  if(!usb.begin())
  {
    Serial.println("Cannot connect to STUSB4500.");
    while(1);
  }
  usb_pd_config();

  // setup PWM
  ledcSetup(PWM_CH_BLUE, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_GREEN, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_RED, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_WW, PWM_FREQ, PWM_RES);
  ledcSetup(PWM_CH_CW, PWM_FREQ, PWM_RES);
  ledcAttachPin(PIN_BLUE, PWM_CH_BLUE);
  ledcAttachPin(PIN_GREEN, PWM_CH_GREEN);
  ledcAttachPin(PIN_RED, PWM_CH_RED);
  ledcAttachPin(PIN_WW, PWM_CH_WW);
  ledcAttachPin(PIN_CW, PWM_CH_CW);
  ledcWrite(PWM_CH_BLUE, 0);
  ledcWrite(PWM_CH_GREEN, 0);
  ledcWrite(PWM_CH_RED, 0);
  ledcWrite(PWM_CH_WW, 0);
  ledcWrite(PWM_CH_CW, 0);
}

void loop() {
}
