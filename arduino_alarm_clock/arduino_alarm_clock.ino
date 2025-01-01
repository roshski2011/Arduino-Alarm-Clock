#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// RTC and LCD
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust I2C address (0x27) if needed

// Pin Definitions
const int buttonConfirm = 2;  // Confirm button
const int buttonHours = 3;    // Hours button
const int buttonMinutes = 4;  // Minutes button
const int buzzer = 5;         // Buzzer

// Variables for Time Setting
int selectedHours = 0;
int selectedMinutes = 0;

// Timer variables
unsigned long startTime = 0;
unsigned long timerDuration = 0; // in milliseconds
bool timerActive = false;

// Debouncing
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50; // 50ms debounce delay

// Button states
bool lastButtonConfirm = LOW;
bool lastButtonHours = LOW;
bool lastButtonMinutes = LOW;

// Function prototypes
void updateLCD(DateTime now);
void beepBuzzer();
void showTimerSetMessage();

void setup() {
  // Initialize LCD
  lcd.init();
  lcd.backlight();

  // Initialize RTC
  if (!rtc.begin()) {
    lcd.print("RTC Not Found!");
    while (1);
  }
  if (rtc.lostPower()) {
    lcd.print("RTC Resetting...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Set to compile time
  }

  // Set up pins
  pinMode(buttonConfirm, INPUT_PULLUP);
  pinMode(buttonHours, INPUT_PULLUP);
  pinMode(buttonMinutes, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);

  lcd.clear();
  lcd.print("Alarm Clock Ready");
  delay(2000);
  lcd.clear();
}

void loop() {
  DateTime now = rtc.now();

  // Display current time on LCD while idle or timer running
  if (!timerActive || (timerActive && millis() - startTime < timerDuration)) {
    updateLCD(now);
  }

  // Read button states
  bool currentConfirm = digitalRead(buttonConfirm);
  bool currentHours = digitalRead(buttonHours);
  bool currentMinutes = digitalRead(buttonMinutes);

  // Handle Hours Button
  if (currentHours == LOW && lastButtonHours == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
    selectedHours = (selectedHours % 24) + 1; // Cycle through 1-24
    lcd.clear();
    lcd.print("Set Hours: ");
    lcd.print(selectedHours);
    lastDebounceTime = millis();
  }

  // Handle Minutes Button
  if (currentMinutes == LOW && lastButtonMinutes == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
    switch (selectedMinutes) {
      case 0: selectedMinutes = 15; break;
      case 15: selectedMinutes = 30; break;
      case 30: selectedMinutes = 45; break;
      case 45: selectedMinutes = 0; break;
    }
    lcd.clear();
    lcd.print("Set Minutes: ");
    lcd.print(selectedMinutes);
    lastDebounceTime = millis();
  }

  // Handle Confirm Button
  if (currentConfirm == LOW && lastButtonConfirm == HIGH && (millis() - lastDebounceTime > debounceDelay)) {
    timerDuration = (selectedHours * 3600000UL) + (selectedMinutes * 60000UL); // Convert to milliseconds
    if (timerDuration > 0) {
      timerActive = true;
      startTime = millis();
      showTimerSetMessage();
      delay(2000); // Show timer set message for 2 seconds
    }
    lastDebounceTime = millis();
  }

  // Timer Logic
  if (timerActive && (millis() - startTime >= timerDuration)) {
    lcd.clear();
    lcd.print("Time's Up!");
    beepBuzzer();
    timerActive = false;
    selectedHours = 0;
    selectedMinutes = 0; // Reset values
    delay(2000);
    lcd.clear();
  }

  // Save last button states
  lastButtonConfirm = currentConfirm;
  lastButtonHours = currentHours;
  lastButtonMinutes = currentMinutes;
}

void updateLCD(DateTime now) {
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  if (now.hour() < 10) lcd.print('0');
  lcd.print(now.hour());
  lcd.print(':');
  if (now.minute() < 10) lcd.print('0');
  lcd.print(now.minute());
  lcd.print(':');
  if (now.second() < 10) lcd.print('0');
  lcd.print(now.second());

  lcd.setCursor(0, 1);
  lcd.print("Set: ");
  lcd.print(selectedHours);
  lcd.print("H ");
  lcd.print(selectedMinutes);
  lcd.print("M");
}

void beepBuzzer() {
  for (int i = 0; i < 5; i++) { // Beep 5 times
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
  }
}

void showTimerSetMessage() {
  lcd.clear();
  lcd.print("Timer Set: ");
  lcd.print(selectedHours);
  lcd.print("H ");
  lcd.print(selectedMinutes);
  lcd.print("M");
}
