#include <LiquidCrystal.h>

// Pins for LCD
const int RS_PIN = 2;
const int E_PIN = 3;
const int D4_PIN = 5;
const int D5_PIN = 6;
const int D6_PIN = 7;
const int D7_PIN = 8;

const int FIX_LENGTH = 8;       // Number of chars that don't scroll
const String SEPARATOR = " * ";

LiquidCrystal lcd(RS_PIN, E_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

// Two lines
// The first string is the fixed part
// The second string is the scrolling part
String lines[2][2] = {{"", ""}, {"", ""}};

int offset1 = 0;
int offset2 = 0;

String displayString1 = "";
String displayString2 = "";

unsigned long updateTime = 0;
int updateRate = 500;

void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  Serial.setTimeout(100);
}

void loop() {
  if (Serial.available() > 0) {   // If there is new data, receive it
    String oldLine1 = lines[0][1];
    String oldLine2 = lines[1][1];

    String input = Serial.readString();
    parseData(input);

    if (lines[0][1] != oldLine1) {
      offset1 = 0;
      updateDisplayStringAndOffset(lines[0][1], &displayString1, &offset1);
      if (lines[0][1].length() > 16 - FIX_LENGTH) {
        updateTime = millis() + 2000;
      }
    }
    if (lines[1][1] != oldLine2) {
      offset2 = 0;
      updateDisplayStringAndOffset(lines[1][1], &displayString2, &offset2);
      if (lines[1][1].length() > 16 - FIX_LENGTH) {
        updateTime = millis() + 2000;
      }
    }

    lcd.clear();
    lcd.print(lines[0][0]);
    lcd.print(displayString1);
    lcd.setCursor(0, 1);
    lcd.print(lines[1][0]);
    lcd.print(displayString2);
  } else {                          // If no new data, scroll in case of need
    if (millis() > updateTime) {
      if (lines[0][1].length() > 16 - FIX_LENGTH) {
        updateDisplayStringAndOffset(lines[0][1], &displayString1, &offset1);
        lcd.setCursor(FIX_LENGTH, 0);
        lcd.print(displayString1);
      }

      if (lines[1][1].length() > 16 - FIX_LENGTH) {
        updateDisplayStringAndOffset(lines[1][1], &displayString2, &offset2);
        lcd.setCursor(FIX_LENGTH, 1);
        lcd.print(displayString2);
      }

      updateTime = updateTime + updateRate;
    }
  }
}

void parseData(String input) {
  String parts[4];
  int p = 0;
  for (int i = 0; i < input.length() && p < 4; i++) {
    if (input[i] == ';') {
      p++;
    } else {
      parts[p] += input[i];
    }
  }

  lines[0][0] = parts[0];
  lines[0][1] = parts[1];
  lines[1][0] = parts[2];
  lines[1][1] = parts[3];
  Serial.println(input);
}

void updateDisplayStringAndOffset(String line, String* displayString, int* offset) {
  if (line.length() <= 16 - FIX_LENGTH) {
    *displayString = line;
  } else {
    int currentOffset = *offset;
    *offset = *offset + 1;
    if (*offset == line.length() + SEPARATOR.length()) {
      *offset = 0;
    }

    if (currentOffset + 16 - FIX_LENGTH > line.length()) {
      line = line + SEPARATOR + line;
    }

    *displayString = line.substring(currentOffset, currentOffset + 16 - FIX_LENGTH);
  }
}
