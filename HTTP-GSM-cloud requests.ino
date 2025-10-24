/*
 * OPTIMIZED HTTP-GSM Cloud Requests System
 * 
 * This sketch implements optimized GSM AT command handling for direct
 * HTTP communication with cloud platforms. Features state machine-based
 * AT command processing, automatic error recovery, and efficient data
 * transmission to ThingSpeak.
 * 
 * Hardware Requirements:
 * - Arduino with SoftwareSerial support
 * - GSM SIM900A module
 * - GSM antenna
 * - SIM card with active data plan
 * - Sensor connected to A0
 * 
 * Key Features:
 * - Non-blocking AT command state machine
 * - Automatic connection establishment and recovery
 * - Optimized data collection and transmission
 * - Memory monitoring and optimization
 * - Robust error handling with retry logic
 * - Direct HTTP communication without libraries
 * 
 * AT Command Flow:
 * 1. Basic GSM module testing
 * 2. SIM card and network verification
 * 3. GPRS connection establishment
 * 4. TCP connection to ThingSpeak
 * 5. Data transmission and monitoring
 * 
 * Original Code: Modified from various GSM tutorials
 */

#include <SoftwareSerial.h>
#include <String.h>
#include "GSM_Utils.h"

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

SoftwareSerial mySerial(7, 8);  // RX, TX pins for GSM module communication

// ============================================================================
// GLOBAL OBJECTS AND STATE MANAGEMENT
// ============================================================================

SensorBuffer sensorBuffer;                      // Efficient sensor data storage
NonBlockingDelay commandDelay(1000);            // Delay between AT commands
NonBlockingDelay dataDelay(100);                // Delay between sensor readings
bool isConnected = false;                        // TCP connection status flag
int commandStep = 0;                             // Current AT command step

// ============================================================================
// SETUP FUNCTION
// ============================================================================

/**
 * Initialize GSM module and start connection process
 * Sets up serial communication and begins AT command sequence
 */
void setup()
{
  // Initialize serial communication
  mySerial.begin(9600);  // GSM module baud rate
  Serial.begin(9600);    // Serial monitor baud rate
  
  // Display system information
  Serial.println("==========================================");
  Serial.println("OPTIMIZED HTTP-GSM Cloud Requests System");
  Serial.println("==========================================");
  Serial.println("Features:");
  Serial.println("- Non-blocking AT command processing");
  Serial.println("- Automatic error recovery");
  Serial.println("- Direct HTTP communication");
  Serial.println("- Memory monitoring");
  Serial.println("==========================================");
  
  // Initialize timing delays
  commandDelay.reset();
  dataDelay.reset();
  
  // Start GSM connection process
  initializeGSMConnection();
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

/**
 * Main program loop - handles GSM state machine and data collection
 * Uses non-blocking approach for continuous operation
 */
void loop()
{
  // Process GSM connection state machine
  handleGSMConnection();
  
  // Collect and send sensor data when connected
  if (isConnected && dataDelay.isComplete()) {
    collectAndSendData();
    dataDelay.reset();
  }
  
  // Handle incoming GSM data and responses
  if (mySerial.available()) {
    char c = mySerial.read();
    Serial.write(c);  // Echo to serial monitor
    handleGSMResponse(c);
  }
  
  // Monitor memory usage periodically (every 30 seconds)
  static NonBlockingDelay memoryPrintDelay(30000);
  if (memoryPrintDelay.isComplete()) {
    printMemoryUsage();
    memoryPrintDelay.reset();
  }
}

// ============================================================================
// GSM CONNECTION MANAGEMENT FUNCTIONS
// ============================================================================

/**
 * Initialize GSM module with comprehensive testing
 * Performs basic communication test, SIM card check, and network verification
 */
void initializeGSMConnection() {
  Serial.println("Testing GSM module...");
  
  // Test basic GSM module communication
  if (sendATCommand(mySerial, "AT", "OK", 2000)) {
    Serial.println("✓ GSM module responding");
  } else {
    Serial.println("✗ GSM module not responding");
    return;
  }
  
  // Check SIM card status
  if (sendATCommand(mySerial, "AT+CPIN?", "READY", 3000)) {
    Serial.println("✓ SIM card ready");
  } else {
    Serial.println("✗ SIM card not ready");
    return;
  }
  
  // Check network registration status
  if (sendATCommand(mySerial, "AT+CREG?", "+CREG: 0,1", 5000)) {
    Serial.println("✓ Network registered");
  } else {
    Serial.println("✗ Network not registered");
    return;
  }
  
  // Check GPRS attachment status
  if (sendATCommand(mySerial, "AT+CGATT?", "+CGATT: 1", 5000)) {
    Serial.println("✓ GPRS attached");
  } else {
    Serial.println("✗ GPRS not attached");
    return;
  }
  
  Serial.println("✓ GSM initialization complete");
}

/**
 * Handle GSM connection state machine
 * Manages AT command sequence for establishing TCP connection
 * Uses non-blocking approach with automatic retry logic
 */
void handleGSMConnection() {
  if (commandDelay.isComplete()) {
    switch(commandStep) {
      case 0: // Shutdown any existing connection
        if (sendATCommand(mySerial, "AT+CIPSHUT", "SHUT OK", 3000)) {
          commandStep = 1;
          commandDelay.reset(2000);
        } else {
          commandStep = 1; // Continue even if shutdown fails
          commandDelay.reset(1000);
        }
        break;
        
      case 1: // Set single IP connection mode
        if (sendATCommand(mySerial, "AT+CIPMUX=0", "OK", 2000)) {
          commandStep = 2;
          commandDelay.reset(2000);
        } else {
          Serial.println("✗ Failed to set single IP connection");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(5000);
        }
        break;
        
      case 2: // Start task and set APN
        if (sendATCommand(mySerial, "AT+CSTT=\"zonginternet\"", "OK", 3000)) {
          commandStep = 3;
          commandDelay.reset(2000);
        } else {
          Serial.println("✗ Failed to set APN");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(5000);
        }
        break;
        
      case 3: // Bring up wireless connection
        if (sendATCommand(mySerial, "AT+CIICR", "OK", 10000)) {
          commandStep = 4;
          commandDelay.reset(2000);
        } else {
          Serial.println("✗ Failed to bring up connection");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(5000);
        }
        break;
        
      case 4: // Get local IP address
        if (sendATCommand(mySerial, "AT+CIFSR", "OK", 5000)) {
          commandStep = 5;
          commandDelay.reset(2000);
        } else {
          Serial.println("✗ Failed to get IP address");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(5000);
        }
        break;
        
      case 5: // Set send prompt mode
        if (sendATCommand(mySerial, "AT+CIPSPRT=0", "OK", 2000)) {
          commandStep = 6;
          commandDelay.reset(2000);
        } else {
          Serial.println("✗ Failed to set send prompt");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(5000);
        }
        break;
        
      case 6: // Start TCP connection to ThingSpeak
        if (sendATCommand(mySerial, "AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",\"80\"", "CONNECT OK", 10000)) {
          Serial.println("✓ TCP connection established");
          isConnected = true;
          commandStep = 7;
          commandDelay.reset(1000);
        } else {
          Serial.println("✗ Failed to establish TCP connection");
          commandStep = 0; // Retry from beginning
          commandDelay.reset(10000);
        }
        break;
        
      case 7: // Connection established, ready for data transmission
        // Stay in this state until connection is lost
        break;
    }
  }
}

// ============================================================================
// DATA COLLECTION AND TRANSMISSION FUNCTIONS
// ============================================================================

/**
 * Collect sensor data and send to ThingSpeak
 * Gathers multiple sensor readings and transmits them efficiently
 */
void collectAndSendData() {
  // Collect 54 sensor samples
  for (int i = 0; i < 54; i++) {
    int value = analogRead(A0);  // Read sensor value
    sensorBuffer.addSample(value);
  }
  
  // Prepare data string for transmission
  char dataString[500];
  sensorBuffer.getDataAsString(dataString, sizeof(dataString));
  
  // Send data to ThingSpeak
  sendDataToThingSpeak(dataString);
  
  // Clear buffer after transmission
  sensorBuffer.clear();
}

/**
 * Send sensor data to ThingSpeak cloud platform
 * Constructs HTTP request and transmits data via GSM module
 * @param data - Sensor data string to transmit
 */
void sendDataToThingSpeak(const char* data) {
  if (!isConnected) {
    Serial.println("✗ Not connected to server");
    return;
  }
  
  // Prepare HTTP request with sensor data
  char httpRequest[600];
  snprintf(httpRequest, sizeof(httpRequest), 
           "GET http://api.thingspeak.com/update?api_key=%s&field1=%s HTTP/1.1\r\n"
           "Host: api.thingspeak.com\r\n"
           "Connection: close\r\n\r\n",
           WRITE_API_KEY, data);
  
  // Send data length command
  char lengthCmd[20];
  snprintf(lengthCmd, sizeof(lengthCmd), "AT+CIPSEND=%d", strlen(httpRequest));
  
  if (sendATCommand(mySerial, lengthCmd, ">", 3000)) {
    // Send HTTP request
    mySerial.print(httpRequest);
    mySerial.println((char)26); // Send character (CTRL+Z)
    
    // Wait for transmission confirmation
    if (waitForResponse(mySerial, "SEND OK", 10000)) {
      Serial.println("✓ Data sent successfully");
    } else {
      Serial.println("✗ Failed to send data");
      isConnected = false;
      commandStep = 0; // Restart connection process
    }
  } else {
    Serial.println("✗ Failed to prepare data send");
    isConnected = false;
    commandStep = 0; // Restart connection process
  }
}

/**
 * Handle incoming GSM responses
 * Processes data from GSM module and updates system state
 * @param c - Character received from GSM module
 */
void handleGSMResponse(char c) {
  // Add specific response handling logic here if needed
  // For now, responses are echoed to serial monitor in main loop
}
