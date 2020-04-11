#include <LiquidCrystal.h>

const int RS_PIN = 2;
const int E_PIN = 3;
const int D4_PIN = 5;
const int D5_PIN = 6;
const int D6_PIN = 7;
const int D7_PIN = 8;

const String SEPARATOR = "  **  ";

LiquidCrystal lcd(RS_PIN, E_PIN, D4_PIN, D5_PIN, D6_PIN, D7_PIN);

String line1 = "Hello World!";
String line2 = "";

int offset1 = 0;
int offset2 = 0;

String displayString1 = line1;
String displayString2 = line2;

unsigned long updateTime1 = 0;
unsigned long updateTime2 = 0;

int updateRate1 = 0;
int updateRate2 = 0;

void setup() {
  // put your setup code here, to run once:
  lcd.begin(16, 2);
  lcd.print(line1);

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 0) {
    String input = Serial.readString();

    String lastTwoLines[] = {"", ""};
    getLastTwoLines(input, lastTwoLines);
    for (int i = 0; i < 2; i++) {
      String line = lastTwoLines[i];
      if (line == "") {
        break;
      }

      if (line2 != "") {
        line1 = line2;
        offset1 = offset2;
        displayString1 = displayString2;
        updateTime1 = updateTime2;
        updateRate1 = updateRate2;
      }
      
      line2 = line;
      offset2 = 0;
      updateDisplayStringAndOffset(line2, &displayString2, &offset2);
      updateTime2 = 0;
      updateRate2 = calculateUpdateRate(line2.length());
    }

    lcd.clear();
    lcd.print(displayString1);
    lcd.setCursor(0, 1);
    lcd.print(displayString2);

    if (line1.length() > 16) {
      updateTime1 = millis() + 2000;
    }

    if (line2.length() > 16) {
      updateTime2 = millis() + 2000;
    }
  } else {
    if (updateTime1 != 0 && millis() > updateTime1) {
      updateDisplayStringAndOffset(line1, &displayString1, &offset1);
      lcd.setCursor(0, 0);
      lcd.print(displayString1);
      updateTime1 = updateTime1 + updateRate1;
    }

    if (updateTime2 != 0 && millis() > updateTime2) {
      updateDisplayStringAndOffset(line2, &displayString2, &offset2);
      lcd.setCursor(0, 1);
      lcd.print(displayString2);
      updateTime2 = updateTime2 + updateRate2;
    }
  }
}

void getLastTwoLines(String input, String* output) {
  input = "\n" + input;
  String line = "";
  for (int i = input.length() - 1; i >= 0; i--) {
    if (input[i] != '\n') {
      line = input[i] + line;
    } else if (line.length() > 0) {
      if (output[0] == "") {
        output[0] = line;
        line = "";
      } else {
        output[1] = output[0];
        output[0] = line;
        break;
      }
    }
  }
}

void updateDisplayStringAndOffset(String line, String* displayString, int* offset) {
  if (line.length() <= 16) {
    *displayString = line;
  } else {
    int currentOffset = *offset;
    *offset = *offset + 1;
    if (*offset == line.length() + SEPARATOR.length()) {
      *offset = 0;
    }

    if (currentOffset + 16 > line.length()) {
      line = line + SEPARATOR + line;
    }

    *displayString = line.substring(currentOffset, currentOffset + 16);
  }
}

int calculateUpdateRate(int lineLength) {
  return 400;
  
  if (lineLength <= 16) {
    return 0;
  }

  int rate = 15000 / lineLength;
  Serial.println(lineLength);
  Serial.println(rate);

  const int MIN_RATE = 300;
  const int MAX_RATE = 1000;
  if (rate < MIN_RATE) {
    return MIN_RATE;
  } else if (rate > MAX_RATE) {
    return MAX_RATE;
  } else {
    return rate;
  }
}
