/*
  ============================================================
                LUMENX
       Predictive Smart Street Lighting
  ============================================================

  Project:
  Centralized 3-Pole Smart Street Light

  Controller:
  ESP32 WROOM-32 / ESP32 DevKit V1 38-pin

  Features:
  - 3 PIR motion sensors
  - 3 independently controlled lights
  - LDR day/night detection
  - PWM brightness control
  - Predictive neighboring-pole illumination
  - Automatic return to dim mode
  - Smooth brightness transition
  - Serial monitoring

  ============================================================
*/

// ============================================================
// PIN CONFIGURATION
// ============================================================

// ---------- PIR SENSORS ----------
const int PIR1_PIN = 26;
const int PIR2_PIN = 27;
const int PIR3_PIN = 14;

// ---------- LDR ----------
const int LDR_PIN = 34;

// ---------- STREET LIGHT OUTPUTS ----------
const int LED1_PIN = 25;
const int LED2_PIN = 33;
const int LED3_PIN = 32;


// ============================================================
// BRIGHTNESS SETTINGS
// ============================================================

// PWM range = 0 to 255

const int DIM_LEVEL        = 90;   // ~35%
const int PREDICTIVE_LEVEL = 180;  // ~70%
const int BRIGHT_LEVEL     = 255;  // 100%
const int OFF_LEVEL        = 0;


// ============================================================
// TIMING SETTINGS
// ============================================================

// Light remains bright/predictive for 6 seconds
// after the last motion detection.

const unsigned long MOTION_TIMEOUT = 6000;

// Smooth brightness change
const int FADE_STEP = 5;

// Delay between control loops
const int LOOP_DELAY = 30;


// ============================================================
// LDR SETTINGS
// ============================================================

// IMPORTANT:
// This value must be calibrated for your LDR module.
//
// Check Serial Monitor during:
// 1. Bright daylight
// 2. Darkness
//
// Then adjust this value.

const int NIGHT_THRESHOLD = 2000;


// ============================================================
// LIGHT STATE VARIABLES
// ============================================================

// Current brightness of each pole
int currentBrightness[3] = {
  DIM_LEVEL,
  DIM_LEVEL,
  DIM_LEVEL
};

// Desired brightness of each pole
int targetBrightness[3] = {
  DIM_LEVEL,
  DIM_LEVEL,
  DIM_LEVEL
};


// Last time motion was detected for each pole
unsigned long lastMotionTime[3] = {
  0,
  0,
  0
};


// ============================================================
// SETUP
// ============================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  Serial.println();
  Serial.println("========================================");
  Serial.println("              LUMENX");
  Serial.println(" Predictive Smart Street Lighting");
  Serial.println("========================================");

  // ---------- PIR ----------
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(PIR3_PIN, INPUT);

  // ---------- LDR ----------
  pinMode(LDR_PIN, INPUT);

  // ---------- LED OUTPUTS ----------
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);

  // ---------- INITIAL LIGHT ----------
  analogWrite(LED1_PIN, DIM_LEVEL);
  analogWrite(LED2_PIN, DIM_LEVEL);
  analogWrite(LED3_PIN, DIM_LEVEL);

  Serial.println("System initialized.");
  Serial.println("Starting monitoring...");
  Serial.println();
}


// ============================================================
// FUNCTION: Set brightness of a particular pole
// ============================================================

void setTargetBrightness(int pole, int brightness)
{
  targetBrightness[pole] = brightness;

  // Keep brightness within valid PWM range
  if (targetBrightness[pole] > 255)
    targetBrightness[pole] = 255;

  if (targetBrightness[pole] < 0)
    targetBrightness[pole] = 0;
}


// ============================================================
// FUNCTION: Handle motion at Pole 1
// ============================================================

void motionPole1()
{
  lastMotionTime[0] = millis();
  lastMotionTime[1] = millis();

  // Pole 1 = Active
  setTargetBrightness(0, BRIGHT_LEVEL);

  // Pole 2 = Predictive
  setTargetBrightness(1, PREDICTIVE_LEVEL);

  Serial.println(">>> MOTION AT POLE 1");
  Serial.println("    Pole 1 = 100%");
  Serial.println("    Pole 2 = 70%  [PREDICTIVE]");
  Serial.println("    Pole 3 = 35%");
}


// ============================================================
// FUNCTION: Handle motion at Pole 2
// ============================================================

void motionPole2()
{
  lastMotionTime[0] = millis();
  lastMotionTime[1] = millis();
  lastMotionTime[2] = millis();

  // Pole 1 = Normal
  setTargetBrightness(0, DIM_LEVEL);

  // Pole 2 = Active
  setTargetBrightness(1, BRIGHT_LEVEL);

  // Pole 3 = Predictive
  setTargetBrightness(2, PREDICTIVE_LEVEL);

  Serial.println(">>> MOTION AT POLE 2");
  Serial.println("    Pole 1 = 35%");
  Serial.println("    Pole 2 = 100%");
  Serial.println("    Pole 3 = 70%  [PREDICTIVE]");
}


// ============================================================
// FUNCTION: Handle motion at Pole 3
// ============================================================

void motionPole3()
{
  lastMotionTime[1] = millis();
  lastMotionTime[2] = millis();

  // Pole 1 = Normal
  setTargetBrightness(0, DIM_LEVEL);

  // Pole 2 = Predictive
  setTargetBrightness(1, PREDICTIVE_LEVEL);

  // Pole 3 = Active
  setTargetBrightness(2, BRIGHT_LEVEL);

  Serial.println(">>> MOTION AT POLE 3");
  Serial.println("    Pole 1 = 35%");
  Serial.println("    Pole 2 = 70%  [PREDICTIVE]");
  Serial.println("    Pole 3 = 100%");
}


// ============================================================
// FUNCTION: Return expired poles to dim mode
// ============================================================

void updateTimeouts()
{
  unsigned long currentTime = millis();

  for (int i = 0; i < 3; i++)
  {
    if (currentTime - lastMotionTime[i] > MOTION_TIMEOUT)
    {
      setTargetBrightness(i, DIM_LEVEL);
    }
  }
}


// ============================================================
// FUNCTION: Smooth brightness transition
// ============================================================

void updateBrightness()
{
  // ---------- POLE 1 ----------
  if (currentBrightness[0] < targetBrightness[0])
  {
    currentBrightness[0] += FADE_STEP;

    if (currentBrightness[0] > targetBrightness[0])
      currentBrightness[0] = targetBrightness[0];
  }
  else if (currentBrightness[0] > targetBrightness[0])
  {
    currentBrightness[0] -= FADE_STEP;

    if (currentBrightness[0] < targetBrightness[0])
      currentBrightness[0] = targetBrightness[0];
  }


  // ---------- POLE 2 ----------
  if (currentBrightness[1] < targetBrightness[1])
  {
    currentBrightness[1] += FADE_STEP;

    if (currentBrightness[1] > targetBrightness[1])
      currentBrightness[1] = targetBrightness[1];
  }
  else if (currentBrightness[1] > targetBrightness[1])
  {
    currentBrightness[1] -= FADE_STEP;

    if (currentBrightness[1] < targetBrightness[1])
      currentBrightness[1] = targetBrightness[1];
  }


  // ---------- POLE 3 ----------
  if (currentBrightness[2] < targetBrightness[2])
  {
    currentBrightness[2] += FADE_STEP;

    if (currentBrightness[2] > targetBrightness[2])
      currentBrightness[2] = targetBrightness[2];
  }
  else if (currentBrightness[2] > targetBrightness[2])
  {
    currentBrightness[2] -= FADE_STEP;

    if (currentBrightness[2] < targetBrightness[2])
      currentBrightness[2] = targetBrightness[2];
  }
}


// ============================================================
// FUNCTION: Send brightness to LEDs
// ============================================================

void outputBrightness(bool nightMode)
{
  if (nightMode)
  {
    analogWrite(LED1_PIN, currentBrightness[0]);
    analogWrite(LED2_PIN, currentBrightness[1]);
    analogWrite(LED3_PIN, currentBrightness[2]);
  }
  else
  {
    // Daytime = all lights OFF
    analogWrite(LED1_PIN, OFF_LEVEL);
    analogWrite(LED2_PIN, OFF_LEVEL);
    analogWrite(LED3_PIN, OFF_LEVEL);
  }
}


// ============================================================
// MAIN LOOP
// ============================================================

void loop()
{
  // ==========================================================
  // 1. READ LDR
  // ==========================================================

  int lightLevel = analogRead(LDR_PIN);

  bool isNight = false;

  if (lightLevel < NIGHT_THRESHOLD)
  {
    isNight = true;
  }


  // ==========================================================
  // 2. READ PIR SENSORS
  // ==========================================================

  bool motion1 = digitalRead(PIR1_PIN);
  bool motion2 = digitalRead(PIR2_PIN);
  bool motion3 = digitalRead(PIR3_PIN);


  // ==========================================================
  // 3. PROCESS MOTION
  // ==========================================================

  if (isNight)
  {
    if (motion1)
    {
      motionPole1();
    }

    if (motion2)
    {
      motionPole2();
    }

    if (motion3)
    {
      motionPole3();
    }
  }


  // ==========================================================
  // 4. RETURN EXPIRED LIGHTS TO DIM
  // ==========================================================

  if (isNight)
  {
    updateTimeouts();
  }
  else
  {
    // Daytime
    setTargetBrightness(0, OFF_LEVEL);
    setTargetBrightness(1, OFF_LEVEL);
    setTargetBrightness(2, OFF_LEVEL);
  }


  // ==========================================================
  // 5. SMOOTH BRIGHTNESS
  // ==========================================================

  updateBrightness();


  // ==========================================================
  // 6. OUTPUT PWM
  // ==========================================================

  outputBrightness(isNight);


  // ==========================================================
  // 7. SERIAL MONITOR
  // ==========================================================

  static unsigned long lastPrint = 0;

  if (millis() - lastPrint > 1000)
  {
    lastPrint = millis();

    Serial.print("LDR = ");
    Serial.print(lightLevel);

    Serial.print(" | Mode = ");

    if (isNight)
      Serial.print("NIGHT");
    else
      Serial.print("DAY");

    Serial.print(" | PIR1 = ");
    Serial.print(motion1);

    Serial.print(" | PIR2 = ");
    Serial.print(motion2);

    Serial.print(" | PIR3 = ");
    Serial.print(motion3);

    Serial.print(" | Brightness = ");

    Serial.print(currentBrightness[0]);
    Serial.print(",");
    Serial.print(currentBrightness[1]);
    Serial.print(",");
    Serial.println(currentBrightness[2]);
  }


  // ==========================================================
  // 8. LOOP DELAY
  // ==========================================================

  delay(LOOP_DELAY);
}
