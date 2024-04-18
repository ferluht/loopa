
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <EncoderTool.h>
using namespace EncoderTool;

#include "CRC8.h"

#define LED_COUNT 6
#define LED_PIN 3
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

#include <MPR121.h>

MPR121 mpr121;

int key_mapping[12] = {6, 8, 10, 11, 9, 7, 5, 4, 2, 0, 1, 3};

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define NOTEOFF_HEADER 128
#define NOTEON_HEADER 144
#define CC_HEADER 176

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// the MIDI channel number to send messages
const int channel = 1;

PolledEncoder enc1;
PolledEncoder enc2;

#define KEY_BOUNCE_TIME 5
#define PAD_BOUNCE_TIME 30

#define S1_PIN 9
#define S2_PIN 10
#define S3_PIN 8

#define E1_PIN 4
#define E2_PIN 2

#define OP_PIN 11
#define OM_PIN 12
#define P1_PIN 17
#define P2_PIN 16
#define P3_PIN 14
#define P4_PIN 13

// Create Bounce objects for each button.  The Bounce object
// automatically deals with contact chatter or "bounce", and
// it makes detecting changes very simple.
Bounce S1 = Bounce(S1_PIN, KEY_BOUNCE_TIME);
Bounce S2 = Bounce(S2_PIN, KEY_BOUNCE_TIME);
Bounce S3 = Bounce(S3_PIN, KEY_BOUNCE_TIME);
Bounce OP = Bounce(OP_PIN, KEY_BOUNCE_TIME);
Bounce OM = Bounce(OM_PIN, KEY_BOUNCE_TIME);   
Bounce E1 = Bounce(E1_PIN, KEY_BOUNCE_TIME);
Bounce E2 = Bounce(E2_PIN, KEY_BOUNCE_TIME);
Bounce P1 = Bounce(P1_PIN, PAD_BOUNCE_TIME);
Bounce P2 = Bounce(P2_PIN, PAD_BOUNCE_TIME);
Bounce P3 = Bounce(P3_PIN, PAD_BOUNCE_TIME);
Bounce P4 = Bounce(P4_PIN, PAD_BOUNCE_TIME);


unsigned char rowbuffer[16];
int rbi = 0;
int row = 0;

void testdrawchar(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.println(F("LOOPA"));
  
  display.display();
}

void process_bounce(Bounce * b, uint8_t h1, uint8_t h2, uint8_t num) {
  if (b->fallingEdge()) {
    Serial.write(h1);
    Serial.write(num);
    Serial.write(127);
    Serial.write('\n');
  }
  if (b->risingEdge()) {
    Serial.write(h2);
    Serial.write(num);
    Serial.write(0);
    Serial.write('\n');
  }
}

void blink() {
  strip.setBrightness(255);
  for (int i = 0; i < 6; i ++) {

    strip.setPixelColor(i, 5 + i * 10, 10, 60 - i * 10);
    if (i > 0) strip.setPixelColor(i - 1, 0, 0, 0);
    strip.show();

    delay(100);

  }
  strip.setPixelColor(5, 0, 0, 0);
  strip.show();
}

void setup() {
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    for(;;); // Don't proceed, loop forever
  }

  for (int i = 0; i < 16; i ++) rowbuffer[i] = 0;

  display.setRotation(0);

  testdrawchar();
  display.clearDisplay();
  display.setCursor(0, 0);

  strip.begin();
  strip.show();

  enc1.begin(6, 7);
  enc1.setLimits(0, 128, false);
  enc1.setValue(64);
  enc2.begin(15, 5);
  enc2.setLimits(0, 128, false);
  enc2.setValue(64);


  pinMode(S1_PIN, INPUT_PULLUP);
  pinMode(S2_PIN, INPUT_PULLUP);
  pinMode(S3_PIN, INPUT_PULLUP);

  pinMode(OP_PIN, INPUT_PULLUP);
  pinMode(OM_PIN, INPUT_PULLUP);

  pinMode(E1_PIN, INPUT_PULLUP);
  pinMode(E2_PIN, INPUT_PULLUP);
  pinMode(P1_PIN, INPUT_PULLUP);
  pinMode(P2_PIN, INPUT_PULLUP);
  pinMode(P3_PIN, INPUT_PULLUP);
  pinMode(P4_PIN, INPUT_PULLUP);

  Serial.begin(115200);

  mpr121.setupSingleDevice(Wire, MPR121::ADDRESS_5A, true);

  mpr121.setAllChannelsThresholds(180, 20);
  mpr121.setDebounce(MPR121::ADDRESS_5A, 1, 1);
  mpr121.setBaselineTracking(MPR121::ADDRESS_5A, MPR121::BASELINE_TRACKING_INIT_10BIT);
  mpr121.setChargeDischargeCurrent(MPR121::ADDRESS_5A, 63);
  mpr121.setChargeDischargeTime(MPR121::ADDRESS_5A, MPR121::CHARGE_DISCHARGE_TIME_HALF_US);
  mpr121.setFirstFilterIterations(MPR121::ADDRESS_5A, MPR121::FIRST_FILTER_ITERATIONS_34);
  mpr121.setSecondFilterIterations(MPR121::ADDRESS_5A, MPR121::SECOND_FILTER_ITERATIONS_10);
  mpr121.setSamplePeriod(MPR121::ADDRESS_5A, MPR121::SAMPLE_PERIOD_1MS);

  mpr121.startChannels(12, MPR121::PROXIMITY_MODE_DISABLED);

  blink();
}

bool boot = true;
unsigned long lastbytetime = 0;
uint16_t prev_status = 0;
uint8_t led_colors[18];
uint8_t crc = 0;

void loop() {

  uint16_t touch_status = mpr121.getTouchStatus(MPR121::ADDRESS_5A);
  for (int i = 0; i < 12; i ++) {
    int s = (touch_status >> i) & 0x01;
    int prev_s = (prev_status >> i) & 0x01;
    if (s != prev_s) {
      if (s > 0) {
        Serial.write(CC_HEADER);
        Serial.write(60 + key_mapping[i]);
        Serial.write(127);
        Serial.write('\n');
      } else {
        Serial.write(CC_HEADER);
        Serial.write(60 + key_mapping[i]);
        Serial.write(0);
        Serial.write('\n');
      }
    }
  }
  prev_status = touch_status;

  
  enc1.tick();
  enc2.tick();

  S1.update();
  S2.update();
  S3.update();
  OP.update();
  OM.update();
  E1.update();
  E2.update();
  P1.update();
  P2.update();
  P3.update();
  P4.update();

  process_bounce(&OM, CC_HEADER, CC_HEADER, 98);
  process_bounce(&OP, CC_HEADER, CC_HEADER, 99);

  process_bounce(&S1, CC_HEADER, CC_HEADER, 100);
  process_bounce(&S2, CC_HEADER, CC_HEADER, 101);
  process_bounce(&S3, CC_HEADER, CC_HEADER, 102);

  process_bounce(&E1, CC_HEADER, CC_HEADER, 107);
  process_bounce(&E2, CC_HEADER, CC_HEADER, 108);

  process_bounce(&P1, CC_HEADER, CC_HEADER, 110);
  process_bounce(&P2, CC_HEADER, CC_HEADER, 111);
  process_bounce(&P3, CC_HEADER, CC_HEADER, 112);
  process_bounce(&P4, CC_HEADER, CC_HEADER, 113);



  if (enc1.valueChanged()) {
    Serial.write(CC_HEADER);
    Serial.write(105);
    Serial.write((enc1.getValue() - 64) * 4 + 64);
    Serial.write('\n');

    enc1.setValue(64);
  }
  if (enc2.valueChanged()) {
    Serial.write(CC_HEADER);
    Serial.write(106);
    Serial.write((enc2.getValue() - 64) * 4 + 64);
    Serial.write('\n');

    enc2.setValue(64);
  }

  while (Serial.available() > 0) {
    unsigned char c = Serial.read();

    if (rbi < 512) {
      rowbuffer[rbi % 16] = c;
      rbi ++;
      
      if (rbi > 0 && (rbi % 16 == 0)) {
        crc = getCrc(crc, rowbuffer, 16);
        display.drawBitmap(0, rbi / 16 - 1, rowbuffer, 128, 1, 1);
      }
    } else {
      if (rbi != 530) {
          led_colors[rbi - 512] = c;
          rbi ++;
      } else {
        for (int i = 0; i < 6; i ++)
          strip.setPixelColor(i, led_colors[i * 3 + 0], led_colors[i * 3 + 1], led_colors[i * 3 + 2]);
        crc = getCrc(crc, led_colors, 18);
        if (crc == c) {
            strip.show();
            display.display();
            display.setCursor(0, 0);
            display.clearDisplay();
        }
        rbi = 0;
        crc = 0;
      }
    }

    lastbytetime = millis();
  }

  if (lastbytetime > 0 && boot) {
    blink();
    boot = false;
  }

  if (lastbytetime > 0 && millis() - lastbytetime > 10) {
      rbi = 0;
      crc = 0;
      display.setCursor(0, 0);
      display.clearDisplay();
  }
}
