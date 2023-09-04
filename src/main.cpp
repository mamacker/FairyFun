#include <Arduino.h>
#include "Adafruit_FreeTouch.h"  // If you don't know about Adafruit
                                 // checkout https://adafruit.com
                                 // it will be your new IOT addiction.

// Author: Matthew Amacker
// Date: 2021-09-25
// License: MIT

// Overview:
// This is built for the Seee Studio XIAO SAMD21 Variant
// which provides an ARM® Cortex®-M0+(SAMD21G18) running at up to 48Hz. 
// In addition to the powerful CPU, it has 256KB Flash and 32KB SRAM 
// on board and supports the USB Type-C interface which can supply 
// power and download code.  This variant is very small, and very cheap
// coming in at under $6 as of 2023.  It does not have any wireless
// capabilities.  But, it is a great little board for simple devices.
//
// This is a program that uses the capacitive touch sensor on the
// SAMD21 board to control the brightness of an LED.  The LED
// is connected to pin 4 (NOODLE_PIN).  The capacitive touch sensor 
// is connected to pin 2 (A0).  The LED is controlled by the
// analogWrite function.  This function takes a value between
// 0 and 255.  0 is off.  255 is full on.  Values in between
// are a percentage of the full on value.  So, 127 is about
// half brightness.  63 is about 25% brightness.  191 is about
// 75% brightness.  The capacitive touch sensor is a little
// more complicated.  It is a sensor that can detect when
// a human finger is touching it.  It is a little more
// complicated than a simple on/off switch.  It can detect
// how close the finger is to the sensor.  This program
// uses that information to control the brightness of the
// LED.  The closer the finger is to the sensor, the brighter
// the LED.  The further away the finger is from the sensor,
// the dimmer the LED.  The LED will also pulse for 30 seconds
// after the user touches the sensor.  This provides a little magic
// to the interaction.  Also, once the user touches the sensor,
// the LED will pulse for a little while, providing a light.
//
// Arduino code is typically single threaded. Which means that
// there is only one thing happening at a time.  Therefore the
// code goes through a very fast loop.  In that loop it checks 
// time passed since last loop and updates state for different 
// things its managing.  For instance - the capacitive touch.
// 
// The theory behind operation:
// Capacitive touch sensors work by measuring the capacitance
// between the sensor and ground.  The human body's outer electric 
// field is a good place where charge can be stored.  Kinda like
// when you rub a balloon on your head and it sticks to the wall.
// So, when a human finger is placed near the sensor, the 
// capacitance between the sensor and ground changes.  This
// is a very, very small change.  But, it is enough to be
// measured with the extremely fast processing time of the CPU
// and the very responsive A2D(Analogue to Digital) converter.
//
// Further, due to the very small changes we are measuring, the 
// physical properties of the device and the environment can make 
// a big difference in the readings.  For instance, if the surface
// of the device is large, it may pickup more stray radio energy
// pre-charging the surface.  Or, if the air is more humid, the time
// it takes for the surface of the device to discharge may be longer.
// There are TONs of reasons the time-to-charge/dischange may be
// different.  So, we need to constantly update the base value
// of the sensor.  This is done by taking a bunch of readings
// and averaging them over time.  This is done in the baseAvg 
// function. 
// 
// Once last note - if you are coming from a web, mobile or 
// desktop development experience - this is different.  In IOT
// devices the power, environment, and physical properties of
// the device are all very important. So, you'll encounter
// more "magic numbers" and other compensation tactics, that
// are normally all hidden away in consumer electronics or cloud
// computers.  This is the beauty of IOT devices.  You get to
// see all the hidden stuff, understand how it works, and explore a 
// more "3D" programming experience.  It is a lot of fun.

// ----

// This library does the pin assignment and sampling of the
// capacitance between the pin and ground.  It takes advantage
// of the analog to digital converter on the board to do the
// sampling.  Generally it measures how long it takes for the
// pin voltage to read HIGH as it fills the capacitance of the
// pin + resitistor combination.  Turns out chips like these have
// resistors you can dynamically attach to the pins.  So, you can
// use the pins as capacitive sensors.  This is a very cool feature
// of the chip.  It is also a very cool feature of the chip that
// you can use the analog to digital converter to sample the
// voltage on the pin.  This is what this library does.  It samples
// the voltage on one of the pins as it turns that pin on and off
// at a specific frequency.  Since it knows the frequency and what it
// expects the voltage to be.  The time it takes to reach that voltage
// is able to be repeatedly measured.  This is the basis of the
// capacitive touch sensing.  The longer it takes to reach the voltage
// the more capacitance there is between the pin and ground.  The
// shorter the time, the less capacitance there is between the pin
// and ground.  Human fingers modify the capacitance between the
// pin and ground.  So, by measuring the time it takes to reach
// the voltage, we can determine if a finger is touching the pin.
// That A0 - means a specific ANALOG READ pin on the board.  There is a mapping
// to real pins hidden away in the .h files attached to this code.  In this
// case A0 is pin 2.  Only some pins have the fancy "analog read" capability.
Adafruit_FreeTouch qt_1 = Adafruit_FreeTouch(A0, OVERSAMPLE_1, RESISTOR_100K, FREQ_MODE_NONE);

// Note - some functions/magic numbers have to be defined before 
// they are used.
// This is because the compiler will not know what the function
// is until it has been defined. This is different from languages
// like Javascript or Python where you can define functions after
// they are used.

// The reason for this is that the compiler is a very simple
// program.  It is not smart enough to know that a function
// will be defined later.  It just knows that it has not been
// defined yet.  So, it will throw an error.  This is why
// we have to define the functions before we use them.

// In Javascript, there is no compiler(kinda - see JIT).  The code 
// is interpreted by the browser in a couple of passes.  The first 
// pass is to find all the functions and variables.  The second 
// pass is to execute the code.  So, in Javascript, you can define 
// functions after they are used.

// Note though - Arduino IDE does something sneaky - which is why
// we can define functions after they are used.  It does a pre-pass
// to find all the functions and variables.  Then it does a second
// pass to execute the code.  So, it is a little more like Javascript
// than C/C++ in this regard... mostly to help out beginners.

#define NUM_BASELINE 5000 // Number of samples to take for the baseline
#define NOODLE_PIN 4 // Where the LED is attached.
#define SPREAD 63 // The range of values between the base and the threshold
                  // that seems to be the most stable for devices I've
                  // tested.  This is a magic number that may need to be
                  // adjusted for different devices.
                  // Note how there is no '=' or ';'
                  // That's because these are drop-in
                  // replacements for the pre-processor.

// Most IOT programs you find will always output their serial
// messages because its easier to always see them.  This device
// lights a blue LED onboard whenever transmissions are done...
// which ruins the magic.  So, we have to go through the pain
// of turning on and off debugging mode.  This is done by
// touching the sensor a specific number of times. 
boolean debugging = true;

// Note - everytime a message is sent over the serial the TX light
// on the board will blink.  This is a good way to know that the
// board is in debug mode.  The trouble is the light blinking is
// annoying when you don't want it.  Which is why we go through
// the pain of going into debug mode.
void checkDebug(int touchTime);

// This is the set of readings that we will use to determine the
// base value of the sensor.  We will take the average of these
// readings to determine the base value.  These are beeing constantly
// updated through the loop function.
int base_readings[NUM_BASELINE];
int base_ct = 0;

// This is the threshold value that we will use to determine if
// the sensor is being touched.  This is updated through the loop
// avg function.
int qt_base = 725;
int qt_Threshold = qt_base + SPREAD;
void lightAtStep();
void lightAtNear(int measurement);

// This is the function that averages in a new base reading.
// Readings will float based on a number of factors.  Moisture
// in the air (a.k.a. the dialectric), temperature, and the
// physical properties of the sensor and even radio energy
// both emitted and absorbed by nearby metals.  So, we need
// to constantly update the base value.
// This is the heart of the "adaptive" or "learning" part.
int baseAvg(int reading)
{
  base_ct++;

  // By using MODULO, we can keep the array index within the
  // bounds of the array. Treating it as a circular buffer.
  base_readings[base_ct % NUM_BASELINE] = reading;

  int sum = 0;
  for (int i = 0; i < NUM_BASELINE; i++)
  {
    sum += base_readings[i];
  }
  return sum / NUM_BASELINE;
}

// the setup function runs once when you press reset or power the board
void setup()
{
  Serial.begin(115200); // Baud rate.  This is the speed at which the serial
                        // port will communicate.  Which boils down to how
                        // fast bit voltages will be toggled on the wires 
                        // used in the TX and RX pins.  This is a very 
                        // common baud rate - decently fast without taxing
                        // the processor too much.  Note - the baud rate
                        // is a property of the serial port.  So, you have
                        // to set it on both the board and the computer.

  Serial.println("Booted"); // Gotta say something.  Note, a lot of times
                            // the serial monitor will not show the first
                            // message.  So, I like to put a message in
                            // the setup function that I know will show
                            // up.  This is a good place to put a
                            // "Booted" message.  When you "reset" the
                            // board, it will show up.  But, when you
                            // first power it up, it will not show up.
                            // There is "println" and "print".  Print
                            // will print the message without a new line.

  // Initialize digital pin LED_BUILTIN as an output. Under the covers
  // this function is setting special registers(like memory addresses) 
  // that cause various transistors connected to the pins to be turned 
  // on and off - this is how the CPU controls the pins and other devices
  // linked to the pins (resitors for example).
  pinMode(NOODLE_PIN, OUTPUT);
  // You might wonder why the LED pin is called "NOODLE" - check out:
  // https://www.adafruit.com/product/5503
  // this is what my fairy boxes use.
  digitalWrite(NOODLE_PIN, 0); // Always good habit to set state. Sometimes
                               // the board will boot with the pin in a
                               // weird state.  So, it is good to set it
                               // to a known state. This is another "value"
                               // stored at a very specific register bit at
                               // a very specific address.

  // This library does some stuff to setup pin resistors and assign timers
  // and stuff.  So, we need to call this function to get it all setup.
  // Note part of this libraries function is using interrupts for very
  // tight measurements.  Timers are interrupts that happen at a very
  // specific time outside of code execution walking with the program
  // counter.
  if (!qt_1.begin())
    Serial.println("Failed to begin qt");

  // At the start of the program, we will init default readings
  // and use the average of these readings to determine the base
  // value of the sensor.
  for (int i = 0; i < NUM_BASELINE; i++)
  {
    base_readings[i] = qt_base; // Magic start value...
  }
}

// The loop function runs over and over again forever, this is
// pretty common and standard for Arduino programs.  It is
// the main loop of the program.  This is where the program
// spends most of its time checking to see if anything has 
// changed and updating the state and timers.
// 
// Other IOT environments might do everything with interrupts 
// or callbacks.  But, Arduino is a very simple environment
// and ensures the code is easier to understand and debug.
// 
// When first getting started in always powered little devices
// it can feel wasteful to have the CPU constantly running
// through the loop.  But, its what these little devices are 
// designed for - so don't worry about it.  If you aren't executing 
// code, like in the little delay at then end of the loop, you save
// tiny amounts of power, but it is not worth the effort to try
// and optimize it.  The CPU is designed to run at full speed
// all the time.  So, let it do its thing.
// 
// When you REALLY want a battery powered device - you'll tell this
// loop to simply delay ALWAYS.  Then you'll use interrupts to
// wake the CPU up when you need it (based on time, or pin event).  
// But, that is a more advanced topic.
// 
#define BASELINE_TIME 5000 // Number of milliseconds to take baseline readings
                           // before starting to do anything.
#define LIGHT_ON_TIME 30000 // Number of milliseconds to keep the light on
                            // after the user touches the sensor.
#define DEBUG_LOOP_COUNT 51 // Number of times through the loop before
                            // printing out the readings.
#define DEBUG_CLEAR_TIME 30000 // Number of milliseconds afterwhich state 
                            // should be reset.
void loop() // Magic function that is called over and over again.
{
  static int touchTime = millis() - DEBUG_CLEAR_TIME; // Start in a "clearable" state.
  int qt1 = 0;
  qt1 = qt_1.measure();

  // Stash a reading... always - this is averaged over the 5000 or so
  // readings for maintaining the baseline in the loop.  Note - the speed
  // these are pulled in a managed by the "delay" at the end of this loop.
  qt_base = baseAvg(qt1);
  qt_Threshold = qt_base + SPREAD; // This magic 63 is the observed range distance
                                   // between the base and the threshold that seems
                                   // to be the most stable for the few devices I've
                                   // tested.  It is possible that this value will
                                   // need to be adjusted for different devices.

  // This is the check to see if the sensor is being touched.
  // If the reading is above the threshold, then the the reading is above 
  // the threshold we belive a finger was needed to get it there.
  if (qt1 >= qt_Threshold)
  {
    if (debugging) 
    {
      Serial.print("Someone touched me!");
      Serial.println(qt1);
    }
    checkDebug(touchTime);
    touchTime = millis(); // Record the timestamp of the touch, and use it later.
  }

  // For the first 5 seconds... just take readings and don't do anything.
  // millis is a function that returns the number of milliseconds since
  // the program/processor started.
  if (millis() < BASELINE_TIME)
  {
    delay(10);
    return;
  }

  // Assuming a touch occurred in the last LIGHT_ON_TIME milliseconds...
  // This function causes the light to pulse for 30 seconds after the
  // user touches the sensor.
  // Note - if no touch has occurred in the last LIGHT_ON_TIME milliseconds
  // then we check to see if the user is NEAR and light based on that nearness.
  if (millis() - touchTime < LIGHT_ON_TIME)
  {
    lightAtStep();
  }
  else
  {
    // This function causes the light to be bright based in proportion to how
    // close the user's finger is to the sensor.
    lightAtNear(qt1);
  }

  // This is just for debugging.  It prints out the readings every 50
  // times through the loop.
  if (base_ct % DEBUG_LOOP_COUNT == 0 && debugging)
  {
    Serial.println("Reading: " + String(qt1));
    Serial.print("Base: ");
    Serial.print(qt_base);
    Serial.print(" Threshold: ");
    Serial.println(qt_Threshold);
  }

  // The side effect of this delay is that it modifies how fast the
  // LED pulses as a results of light at step and light at near.
  delay(10);
}

// Note - normally people like to move DEFINE statements to the top
// of the file.  This is because the compiler does a pre-pass to
// find all the defines and replace them with the value.  So, it
// is a good idea to put them at the top of the file so you can
// find them easily.  But, I like to put them near the code that
// uses them - because it is easier to understand the code when
// the magic numbers are close to the code that uses them.
#define NUM_MEAS 50
int measSet[NUM_MEAS];
int addMeasurement(int measurement)
{
  static int measCount = 0;
  static int avgMeasure = 0;
  measCount++;
  measSet[measCount % NUM_MEAS] = measurement;
  avgMeasure = 0;
  for (int i = 0; i < NUM_MEAS; i++)
  {
    avgMeasure += measSet[i];
  }
  avgMeasure /= NUM_MEAS;
  if (measCount % NUM_MEAS == 0 && debugging)
    Serial.println("Avg: " + String(avgMeasure));
  return avgMeasure;
}

// The purpose of this function is to set a light value to corresponds
// to how "close" the user's finger is to the box.
// The closer the finger, the higher the light value.  But, we have
// to be careful because it needs to be within the range of the
// detection threshold which is always shifting a little.
#define MIN_OVER_THRESHOLD 3
void lightAtNear(int measurement)
{
  static int lastLightMeasure = 0;

  // This checks to see if we like the measurement for "near".
  if (measurement > qt_base + MIN_OVER_THRESHOLD)
  {
    // We have a good measurement.  So, we want to set the light
    // to the value that corresponds to how close the finger is.
    int avgMeasure = addMeasurement(measurement);

    // Slight adjustment to the light value to make it more
    // visible.
    lastLightMeasure = avgMeasure - qt_base;

    // Check to make sure the light value is within the range "on average"
    if (avgMeasure > qt_base + MIN_OVER_THRESHOLD)
      analogWrite(NOODLE_PIN, lastLightMeasure);
  }
  else
  {
    // This is the case where the user's finger is not close enough
    // to the sensor to be detected.  We want to semi-slowly fade the
    // light down to zero.
    int avgMeasure = addMeasurement(0);

    int closingValue = avgMeasure - qt_base;
    if (closingValue > lastLightMeasure)
    {
      closingValue = lastLightMeasure;
    }

    // Without this check the light will go negative and the LED will
    // turn on because the value is interpreted as a very large number.
    if (closingValue < 0)
    {
      closingValue = 0;
    }

    // Check to see if there is still enough value here to have it on.
    // Otherwise, turn it off completely.
    if (avgMeasure > qt_base - MIN_OVER_THRESHOLD)
      analogWrite(NOODLE_PIN, closingValue);
    else
      analogWrite(NOODLE_PIN, 0);
  }
}

// Magic number that determins how many changes in the light
// output there are.  The higher the number, the more gradual
// and longer it will take to get to the minumum or maximum
// brightness. When it reaches the minumum or maximum brightness
// it will reverse direction.
#define NUM_LIGHT_STEPS 150
#define MINUMUM_BRIGHTNESS 10
void lightAtStep()
{
  static int steps = 0; // Static variables exist between function calls
                        // and are initialized once.  This is a good
                        // way to keep state between function calls.
                        // An alternative is to use global variables.
                        // But, static variables are better because
                        // they are only visible to the function that
                        // uses them.  Global variables are visible
                        // to all functions.  So, it is easy to
                        // accidentally change a global variable
                        // and not know where it was changed.
  static bool direction = 1;

  if (steps >= NUM_LIGHT_STEPS)
  {
    direction = false;
  }

  if (steps <= 0)
  {
    direction = true;
  }

  if (direction)
  {
    steps++;
  }
  else
  {
    steps--;
  }

  analogWrite(NOODLE_PIN, (255 - MINUMUM_BRIGHTNESS) * steps / NUM_LIGHT_STEPS + MINUMUM_BRIGHTNESS);
}

#define DEBUG_CHECK_THRESHOLD 5 
#define DEBUG_CHECK_THRESHOLD_MAX 15
#define TOUCH_TIME_DEBOUNCE 300 // Tunable.  How fast do we allow for registering
                                // touches.  This is in milliseconds.
void checkDebug(int touchTime)
{
  // This is used to determine if touch state should be 
  // "Forgotten".  So that debugging will auto age out.
  static unsigned long firstTouch = 0;
  static int debugCheckCt = 0;

  return;

  // This check ensures we only register a touch every 300ms.
  // Sometimes we return so fast, a finger will still be present
  // between touches - this ensures the user has pulled away between
  // counting touches.
  if (millis() - touchTime > TOUCH_TIME_DEBOUNCE)
  {
    bool resetMe = false;
    if (firstTouch == 0 || millis() - firstTouch > DEBUG_CLEAR_TIME)
    {
      resetMe = true;
      firstTouch = millis();
    }

    if (resetMe)
    {
      debugCheckCt = 0;
    }

    debugCheckCt++;

    if (debugCheckCt >= DEBUG_CHECK_THRESHOLD)
    {
      Serial.println("Debugging on.");
      debugging = true;
      // Secret message here. Its between two debug thresholds.
      if (debugCheckCt > 10 && debugCheckCt < DEBUG_CHECK_THRESHOLD_MAX)
      {
        Serial.println("Secret message output here.");
      }
    }
    else
    {
      if (debugging)
        Serial.println("Debugging off.");
      debugging = false;
    }
  }
}
