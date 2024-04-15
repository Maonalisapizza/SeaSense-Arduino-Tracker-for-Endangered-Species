#include <Wire.h> // Include the Wire library for I2C communication
#include <Adafruit_GFX.h> // Include Adafruit's GFX library for graphics
#include <Adafruit_SSD1306.h> // Include Adafruit's SSD1306 OLED library
#include <Ultrasonic.h> // Include Ultrasonic library for interfacing with ultrasonic sensor
#include <Adafruit_NeoPixel.h> // Include Adafruit's NeoPixel library for controlling RGB LEDs

// Define pin numbers for components
#define SEAL_LED_PIN    5  // Pin connected to the SEAL LED strip
#define TOAD_LED_PIN    4  // Pin connected to the TOAD LED strip
#define TURTLE_LED_PIN  3  // Pin connected to the TURTLE LED strip
#define PIEZO_PIN      13  // Pin connected to the piezo buzzer

#define NUM_LEDS_PER_STRIP 12  // Number of LEDs in each strip

// Define variables
Ultrasonic ultrasonic(7, 6); // Define the ultrasonic sensor object. Trigger Pin = 7; Echo Pin = 6

// Initialize OLED display with I2C address 0x3C
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

float waterLevel_detected; // Variable to store the water level
float waterLevel_real;

// Define thresholds for extinction levels
float seal_death_height = 3.0;
float toad_death_height = 5.0;
float turtle_death_height = 8.0;

// Flags to track whether extinction messages have been shown
bool sealDiedShown = false;
bool toadDiedShown = false;
bool turtleDiedShown = false;
bool humanNextShown = false; // Flag to check if "Next... Human" has been shown
bool clearSealMessage = false; // Flag to indicate whether to clear the seal message
bool clearTurtleMessage = false; // Flag to indicate whether to clear the turtle message

// Initialize NeoPixel strips
Adafruit_NeoPixel sealStrip(NUM_LEDS_PER_STRIP, SEAL_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel toadStrip(NUM_LEDS_PER_STRIP, TOAD_LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel turtleStrip(NUM_LEDS_PER_STRIP, TURTLE_LED_PIN, NEO_GRB + NEO_KHZ800);

unsigned long turtleExtinctTime; // Variable to store the time when the turtle message was shown (in order to show the last message)

void setup() {
  Serial.begin(9600); // Initialize serial communication

  // Initialize the OLED display
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // If OLED initialization fails, enter infinite loop
  }

  // Clear the display buffer.
  display.clearDisplay();
  display.display();

  // Set text color to white
  display.setTextColor(SSD1306_WHITE);

  // Set text size
  display.setTextSize(2);

  // Set cursor position
  display.setCursor(0, 0);

  // Initialize LED strips
  sealStrip.begin();
  toadStrip.begin();
  turtleStrip.begin();

  // Initialize all pixels to 'off' for each strip
  sealStrip.show();
  toadStrip.show();
  turtleStrip.show();

  // Set initial color for all LEDs in each strip
  for(int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
    // SEAL strip - Cyan color
    sealStrip.setPixelColor(i, sealStrip.Color(0, 255, 255));
    // TOAD strip - Pink color
    toadStrip.setPixelColor(i, toadStrip.Color(255, 102, 178));
    // TURTLE strip - Yellow color
    turtleStrip.setPixelColor(i, turtleStrip.Color(255, 255, 0));
  }

  sealStrip.show();   // Update SEAL strip
  toadStrip.show();   // Update TOAD strip
  turtleStrip.show(); // Update TURTLE strip
  
  // Initialize piezo pin
  pinMode(PIEZO_PIN, OUTPUT);
}

void loop() {
  // Measure the distance using ultrasonic sensor in centimeters
  waterLevel_detected = ultrasonic.read(CM);
  waterLevel_real = 12.0 - waterLevel_detected; // The height of the container minus the distance between the ultrasonic sensor and the water = the real water level

  // Clear the display buffer.
  display.clearDisplay();

  // Print initial message
  display.setCursor(0, 0);

  // Display messages based on water level
  if (waterLevel_real >= seal_death_height && !sealDiedShown) {
    // Display message for seal extinction
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Hawaiian monk seal has gone extinct");
    sealDiedShown = true; // Set flag to indicate that seal extinction message has been shown
    showExtinctLED(sealStrip, 0, 255, 255); // Shine the seal LED strip
    noTone(PIEZO_PIN); // Stop the tone
  } else if (waterLevel_real >= toad_death_height && !toadDiedShown) {
    // Display message for toad extinction
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Golden toad has gone extinct");
    toadDiedShown = true; // Set flag to indicate that toad extinction message has been shown
    showExtinctLED(toadStrip, 255, 102, 178); // Shine the toad LED strip
    noTone(PIEZO_PIN); // Stop the tone
  } else if (waterLevel_real >= turtle_death_height && !turtleDiedShown) {
    // Display message for turtle extinction
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Green sea turtle has gone extinct");
    turtleDiedShown = true; // Set flag to indicate that turtle extinction message has been shown
    turtleExtinctTime = millis(); // Record the time when turtle message is shown
    showExtinctLED(turtleStrip, 255, 255, 0); // Shine the turtle LED strip
    noTone(PIEZO_PIN); // Stop the tone
  } else if (turtleDiedShown && !humanNextShown) {
    // Display message to prompt the next action after turtle extinction message
    if (millis() - turtleExtinctTime >= 3000) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("Next... Human");
      display.display();
      humanNextShown = true; // Set flag to indicate that "Next... Human" message has been shown
    }
  }

  // Update the display
  display.display();
  
  // Add some delay for stability
  delay(1000);
}

// Function to shine LEDs in a specified color for a certain duration
void showExtinctLED(Adafruit_NeoPixel &strip, uint8_t red, uint8_t green, uint8_t blue) {
  unsigned long startTime = millis();
  while (millis() - startTime < 3000) { // Loop for 3 seconds
    // Update LED strip color
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      strip.setPixelColor(i, strip.Color(red, green, blue)); // Set color
    }
    strip.show();

    // Play sound
    playGameOverSound();
    delay(50);

    // Turn off LED strip
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
      strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off all LEDs
    }
    strip.show();
    delay(50);
  }
}

// Function to play game over sound effect
void playGameOverSound() {
  // Play game over sound effect (siren-like sound)
  tone(PIEZO_PIN, 1000, 100); // Play tone at 1000Hz for 100ms
  delay(50); // Delay for a smooth siren effect
  tone(PIEZO_PIN, 500, 100); // Play tone at 500Hz for 100ms
  delay(50); // Delay for a smooth siren effect
}
