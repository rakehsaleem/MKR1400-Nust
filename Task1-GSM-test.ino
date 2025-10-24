/*
 * OPTIMIZED GSM SIM900A MINI V3.9.2 TEST CODE
 * 
 * This sketch provides comprehensive testing and control of GSM SIM900A module
 * with optimized non-blocking operations and robust error handling.
 * 
 * Hardware Setup:
 * - Connect 5VT to D9 and 5VR to D10
 * - Feed GSM SIM900A with Arduino's 5V
 * - Insert SIM card with active data plan
 * 
 * Features:
 * - Non-blocking GSM operations using state machine
 * - Automatic error recovery and retry logic
 * - Memory monitoring and optimization
 * - Comprehensive AT command handling
 * - SMS sending/receiving capabilities
 * - Voice call management
 * 
 * Original Code by IDAYU SABRI - MYBOTIC
 * Modified by Rakeh - 4 Mar 2018
 * OPTIMIZED for performance and reliability - 2024
 * 
 * Usage:
 * Send commands via Serial Monitor:
 * 's' - Send SMS
 * 'r' - Receive SMS
 * 'c' - Make call
 * 'h' - Hang up call
 * 'e' - Redial last number
 * 'i' - Answer incoming call
 */

#include <SoftwareSerial.h>
#include "GSM_Utils.h"

// ============================================================================
// HARDWARE CONFIGURATION
// ============================================================================

SoftwareSerial mySerial(7, 8);  // RX, TX pins for GSM module communication

// Phone numbers for testing (configure these for your setup)
const char PHONE_NUMBER[] = "+923365926055";  // SMS recipient number
const char CALL_NUMBER[] = "+923335643040";  // Call destination number

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

/**
 * GSM Operation States
 * Defines all possible states for GSM operations
 */
enum GSMState {
  GSM_IDLE,           // Ready for new commands
  GSM_SENDING_SMS,    // SMS transmission in progress
  GSM_RECEIVING_SMS,  // SMS reception in progress
  GSM_MAKING_CALL,    // Outgoing call in progress
  GSM_RECEIVING_CALL, // Incoming call handling
  GSM_HANGING_UP,     // Call termination in progress
  GSM_REDIALING       // Redial operation in progress
};

// Global state variables
GSMState currentState = GSM_IDLE;     // Current operation state
NonBlockingDelay stateDelay(0);       // Timer for state transitions
bool commandInProgress = false;       // Prevents overlapping commands

// ============================================================================
// SETUP FUNCTION
// ============================================================================

/**
 * Initialize GSM module and display usage instructions
 * Performs basic GSM module testing and setup
 */
void setup()
{
  // Initialize serial communication
  mySerial.begin(9600);   // GSM module baud rate
  Serial.begin(9600);      // Serial monitor baud rate
  
  // Display program information and instructions
  Serial.println("==========================================");
  Serial.println("GSM SIM900A OPTIMIZED TEST SYSTEM");
  Serial.println("==========================================");
  Serial.println("Available Commands:");
  Serial.println("  's' - Send SMS message");
  Serial.println("  'r' - Receive SMS message");
  Serial.println("  'c' - Make voice call");
  Serial.println("  'h' - Hang up current call");
  Serial.println("  'e' - Redial last number");
  Serial.println("  'i' - Answer incoming call");
  Serial.println("==========================================");
  
  // Initialize GSM module with comprehensive testing
  initializeGSM();
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

/**
 * Main program loop - handles GSM operations and user input
 * Uses non-blocking approach to maintain system responsiveness
 */
void loop()
{
  // Process GSM state machine for ongoing operations
  handleGSMState();
  
  // Process user commands only when system is idle
  if (currentState == GSM_IDLE && Serial.available() > 0) {
    char command = Serial.read();
    processCommand(command);
  }
  
  // Handle incoming GSM data and responses
  if (mySerial.available() > 0) {
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
// GSM INITIALIZATION AND MANAGEMENT FUNCTIONS
// ============================================================================

/**
 * Initialize GSM module with comprehensive testing
 * Performs basic communication test, SIM card check, and network registration
 */
void initializeGSM() {
  Serial.println("Initializing GSM module...");
  
  // Test basic GSM module communication
  if (sendATCommand(mySerial, "AT", "OK", 2000)) {
    Serial.println("✓ GSM module responding");
  } else {
    Serial.println("✗ GSM module not responding - check connections");
    return;
  }
  
  // Check SIM card status
  if (sendATCommand(mySerial, "AT+CPIN?", "READY", 3000)) {
    Serial.println("✓ SIM card ready");
  } else {
    Serial.println("✗ SIM card not ready - check SIM insertion");
  }
  
  // Check network registration status
  if (sendATCommand(mySerial, "AT+CREG?", "+CREG: 0,1", 5000)) {
    Serial.println("✓ Network registered");
  } else {
    Serial.println("✗ Network not registered - check signal strength");
  }
  
  Serial.println("GSM initialization complete - ready for commands");
}

/**
 * Process user commands from Serial Monitor
 * Validates commands and initiates appropriate GSM operations
 * @param command - Single character command from user
 */
void processCommand(char command) {
  // Prevent overlapping commands
  if (commandInProgress) {
    Serial.println("Command already in progress - please wait");
    return;
  }
  
  // Process valid commands
  switch(command) {
    case 's':
      startSendMessage();
      break;
    case 'r':
      startReceiveMessage();
      break;
    case 'c':
      startMakeCall();
      break;
    case 'h':
      startHangupCall();
      break;
    case 'e':
      startRedialCall();
      break;
    case 'i':
      startReceiveCall();
      break;
    default:
      Serial.println("Unknown command - use 's', 'r', 'c', 'h', 'e', or 'i'");
  }
}

/**
 * Handle GSM state machine transitions
 * Manages ongoing GSM operations using non-blocking state machine
 */
void handleGSMState() {
  // Only process state transitions when delay is complete
  if (stateDelay.isComplete()) {
    switch(currentState) {
      case GSM_SENDING_SMS:
        handleSendMessageState();
        break;
      case GSM_RECEIVING_SMS:
        handleReceiveMessageState();
        break;
      case GSM_MAKING_CALL:
        handleMakeCallState();
        break;
      case GSM_RECEIVING_CALL:
        handleReceiveCallState();
        break;
      case GSM_HANGING_UP:
        handleHangupCallState();
        break;
      case GSM_REDIALING:
        handleRedialCallState();
        break;
      default:
        // GSM_IDLE - no action needed
        break;
    }
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

// ============================================================================
// SMS OPERATION FUNCTIONS
// ============================================================================

/**
 * Start SMS sending process
 * Initiates multi-step SMS transmission using state machine
 */
void startSendMessage() {
  currentState = GSM_SENDING_SMS;
  commandInProgress = true;
  stateDelay.reset(1000);
  mySerial.println("AT+CMGF=1");  // Set text mode
  Serial.println("Starting SMS transmission...");
}

/**
 * Handle SMS sending state machine
 * Manages multi-step SMS process: text mode -> recipient -> message -> send
 */
void handleSendMessageState() {
  static int step = 0;
  
  switch(step) {
    case 0: // Set text mode
      if (waitForResponse(mySerial, "OK", 2000)) {
        step = 1;
        stateDelay.reset(1000);
        char smsCommand[50];
        snprintf(smsCommand, sizeof(smsCommand), "AT+CMGS=\"%s\"", PHONE_NUMBER);
        mySerial.println(smsCommand);
      } else {
        Serial.println("Failed to set text mode");
        resetState();
      }
      break;
      
    case 1: // Send message content
      if (waitForResponse(mySerial, ">", 2000)) {
        step = 2;
        stateDelay.reset(100);
        mySerial.println("Hello, This is an optimized SMS test");
        mySerial.println((char)26); // CTRL+Z to send
      } else {
        Serial.println("Failed to get SMS prompt");
        resetState();
      }
      break;
      
    case 2: // Wait for send confirmation
      if (waitForResponse(mySerial, "OK", 5000)) {
        Serial.println("✓ SMS sent successfully");
        resetState();
      } else {
        Serial.println("✗ SMS send failed");
        resetState();
      }
      break;
  }
}

/**
 * Start SMS receiving process
 * Initiates SMS reading from SIM card
 */
void startReceiveMessage() {
  currentState = GSM_RECEIVING_SMS;
  commandInProgress = true;
  stateDelay.reset(1000);
  mySerial.println("AT+CMGR=1");  // Read first SMS
  Serial.println("Reading SMS from SIM card...");
}

/**
 * Handle SMS receiving state machine
 * Processes SMS reading operation
 */
void handleReceiveMessageState() {
  if (waitForResponse(mySerial, "OK", 3000)) {
    Serial.println("✓ SMS read complete");
    resetState();
  } else {
    Serial.println("✗ Failed to read SMS");
    resetState();
  }
}

// ============================================================================
// VOICE CALL OPERATION FUNCTIONS
// ============================================================================

/**
 * Start voice call process
 * Initiates outgoing call to configured number
 */
void startMakeCall() {
  currentState = GSM_MAKING_CALL;
  commandInProgress = true;
  stateDelay.reset(1000);
  char callCommand[30];
  snprintf(callCommand, sizeof(callCommand), "ATD%s;", CALL_NUMBER);
  mySerial.println(callCommand);
  Serial.println("Initiating voice call...");
}

/**
 * Handle call making state machine
 * Monitors call establishment process
 */
void handleMakeCallState() {
  if (waitForResponse(mySerial, "OK", 10000)) {
    Serial.println("✓ Call initiated successfully");
    resetState();
  } else {
    Serial.println("✗ Call initiation failed");
    resetState();
  }
}

/**
 * Start call receiving process
 * Answers incoming call automatically
 */
void startReceiveCall() {
  currentState = GSM_RECEIVING_CALL;
  commandInProgress = true;
  stateDelay.reset(1000);
  mySerial.println("ATA");  // Answer call
  Serial.println("Answering incoming call...");
}

/**
 * Handle call receiving state machine
 * Processes incoming call answer operation
 */
void handleReceiveCallState() {
  if (waitForResponse(mySerial, "OK", 3000)) {
    Serial.println("✓ Call answered successfully");
    resetState();
  } else {
    Serial.println("✗ Failed to answer call");
    resetState();
  }
}

/**
 * Start call hangup process
 * Terminates current active call
 */
void startHangupCall() {
  currentState = GSM_HANGING_UP;
  commandInProgress = true;
  stateDelay.reset(1000);
  mySerial.println("ATH");  // Hang up call
  Serial.println("Hanging up call...");
}

/**
 * Handle call hangup state machine
 * Processes call termination operation
 */
void handleHangupCallState() {
  if (waitForResponse(mySerial, "OK", 3000)) {
    Serial.println("✓ Call hung up successfully");
    resetState();
  } else {
    Serial.println("✗ Failed to hang up call");
    resetState();
  }
}

/**
 * Start redial process
 * Redials the last called number
 */
void startRedialCall() {
  currentState = GSM_REDIALING;
  commandInProgress = true;
  stateDelay.reset(1000);
  mySerial.println("ATDL");  // Redial last number
  Serial.println("Redialing last number...");
}

/**
 * Handle redial state machine
 * Processes redial operation
 */
void handleRedialCallState() {
  if (waitForResponse(mySerial, "OK", 10000)) {
    Serial.println("✓ Redial initiated successfully");
    resetState();
  } else {
    Serial.println("✗ Redial failed");
    resetState();
  }
}

// ============================================================================
// STATE MANAGEMENT UTILITIES
// ============================================================================

/**
 * Reset GSM state machine to idle
 * Cleans up current operation and prepares for next command
 */
void resetState() {
  currentState = GSM_IDLE;
  commandInProgress = false;
  stateDelay.reset(0);
  clearSerialBuffer(mySerial);
}


