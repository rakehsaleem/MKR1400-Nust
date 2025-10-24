/*
 * OPTIMIZED ThingSpeak Web Client for MKR GSM 1400
 * 
 * This sketch demonstrates optimized IoT data transmission to ThingSpeak cloud platform
 * using MKR GSM 1400 board with cellular connectivity. Features non-blocking operations,
 * automatic reconnection, and efficient data handling.
 * 
 * Hardware Requirements:
 * - MKR GSM 1400 board
 * - GSM antenna
 * - SIM card with active data plan
 * - Optional: Light sensor connected to A0
 * 
 * Key Features:
 * - Non-blocking GSM connection management
 * - Automatic data collection and transmission
 * - Memory-efficient sensor data buffering
 * - Automatic error recovery and reconnection
 * - Real-time memory monitoring
 * - Continuous operation capability
 * 
 * ThingSpeak Integration:
 * - Field 1: Light sensor readings (TEMT6000)
 * - Field 2: LED status indicator
 * - Field 7: GPS latitude (future use)
 * - Field 8: GPS longitude (future use)
 * 
 * Original Code by SMGS - 05 Feb 2018
 * Modified by Rakeh - 23 Apr 2018
 */

// Required libraries
#include <MKRGSM.h>
#include <stdio.h>
#include <string.h>
#include "GSM_Utils.h"

// ============================================================================
// THINGSPEAK FIELD CONFIGURATION
// ============================================================================

// ThingSpeak field definitions for data organization
int t_light_sensor = 1;  // Field 1 - Light sensor readings (TEMT6000)
int t_led_button = 2;    // Field 2 - LED status indicator
int t_latitude = 7;      // Field 7 - GPS latitude (reserved for future use)
int t_longitude = 8;     // Field 8 - GPS longitude (reserved for future use)

// ============================================================================
// GLOBAL OBJECTS AND STATE MANAGEMENT
// ============================================================================

GSMConnection gsmConnection;                    // GSM connection manager
SensorBuffer sensorBuffer;                      // Efficient sensor data storage
NonBlockingDelay dataCollectionDelay(5000);     // Data collection interval (5 seconds)
NonBlockingDelay transmissionDelay(30000);      // Data transmission interval (30 seconds)
bool isInitialized = false;                     // GSM connection status flag

// ============================================================================
// SETUP FUNCTION
// ============================================================================

/**
 * Initialize system and establish GSM connection
 * Sets up hardware, establishes cellular connection, and prepares for operation
 */
void setup(){
  // Configure LED pin for status indication
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB port only)
  }

  // Display system information
  Serial.println("==========================================");
  Serial.println("OPTIMIZED ThingSpeak IoT Client");
  Serial.println("==========================================");
  Serial.println("Features:");
  Serial.println("- Non-blocking GSM operations");
  Serial.println("- Automatic data collection");
  Serial.println("- Memory monitoring");
  Serial.println("- Error recovery");
  Serial.println("==========================================");
  
  // Display initial memory status
  printMemoryUsage();
  
  // Establish GSM connection
  if (gsmConnection.connect()) {
    Serial.println("✓ GSM connection established successfully");
    isInitialized = true;
    } else {
    Serial.println("✗ Failed to establish GSM connection");
    Serial.println("  Check SIM card and signal strength");
    isInitialized = false;
  }
  
  // Initialize timing delays
  dataCollectionDelay.reset();
  transmissionDelay.reset();
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

/**
 * Main program loop - handles data collection and transmission
 * Uses non-blocking approach for continuous operation
 */
void loop(){
  // Attempt reconnection if GSM is not initialized
  if (!isInitialized) {
    if (gsmConnection.connect()) {
      Serial.println("✓ GSM reconnected successfully");
      isInitialized = true;
    } else {
      delay(5000); // Wait 5 seconds before retry
      return;
    }
  }
  
  // Collect sensor data at regular intervals
  if (dataCollectionDelay.isComplete()) {
    collectSensorData();
    dataCollectionDelay.reset();
  }
  
  // Transmit data to ThingSpeak at regular intervals
  if (transmissionDelay.isComplete()) {
    transmitDataToThingSpeak();
    transmissionDelay.reset();
  }
  
  // Handle incoming GSM responses
  gsmConnection.handleResponse();
  
  // Monitor memory usage periodically (every minute)
  static NonBlockingDelay memoryPrintDelay(60000);
  if (memoryPrintDelay.isComplete()) {
    printMemoryUsage();
    memoryPrintDelay.reset();
  }
}

// ============================================================================
// DATA COLLECTION AND TRANSMISSION FUNCTIONS
// ============================================================================

/**
 * Collect sensor data efficiently
 * Reads analog sensor and stores in buffer with LED status indication
 */
void collectSensorData() {
  // Read light sensor value from analog pin A0
  int lightValue = analogRead(A0);
  
  // Store sensor reading in buffer
  sensorBuffer.addSample(lightValue);
  
  // Toggle LED to indicate data collection activity
  digitalWrite(13, !digitalRead(13));
  
  // Display collected data
  Serial.print("Collected sensor data: ");
  Serial.println(lightValue);
}

/**
 * Transmit collected data to ThingSpeak cloud platform
 * Sends current sensor reading and verifies transmission
 */
void transmitDataToThingSpeak() {
  // Check if we have data to transmit
  if (sensorBuffer.getSampleCount() == 0) {
    Serial.println("No data to transmit");
    return;
  }
  
  Serial.println("Transmitting data to ThingSpeak...");
  
  // Get current sensor reading for transmission
  int currentValue = analogRead(A0);
  char url[200];
  
  // Build ThingSpeak URL for data writing
  buildThingSpeakURL(url, t_light_sensor, currentValue, true);
  
  // Send data to ThingSpeak
  if (gsmConnection.sendData(url, "GET")) {
    Serial.println("✓ Data transmitted successfully");
    
    // Verify transmission by reading back data
    char readUrl[200];
    buildThingSpeakReadURL(readUrl, t_light_sensor, 1);
    
    if (gsmConnection.sendData(readUrl, "GET")) {
      Serial.println("✓ Data verification requested");
    }
  } else {
    Serial.println("✗ Failed to transmit data");
    isInitialized = false; // Mark for reconnection
  }
  
  // Clear buffer after successful transmission
  sensorBuffer.clear();
}
