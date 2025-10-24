/*
 * GSM Utilities Library Header
 * Optimized common functions for GSM operations
 * Created for MKR1400-Nust project optimization
 * 
 * This library provides:
 * - Non-blocking GSM connection management
 * - Optimized sensor data buffering
 * - Centralized configuration management
 * - Memory monitoring utilities
 * - ThingSpeak integration helpers
 * 
 * Author: Rakeh Saleem
 * Date: 2024
 */

#ifndef GSM_UTILS_H
#define GSM_UTILS_H

#include <MKRGSM.h>
#include <Arduino.h>

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

// GSM Network Configuration
const char PINNUMBER[] = "";           // SIM card PIN (empty if not required)
const char GPRS_APN[] = "zonginternet"; // APN for your mobile network provider
const char GPRS_LOGIN[] = "";          // GPRS login (usually empty)
const char GPRS_PASSWORD[] = "";       // GPRS password (usually empty)

// ThingSpeak Cloud Platform Configuration
const char THINGSPEAK_SERVER[] = "api.thingspeak.com";
const int THINGSPEAK_PORT = 443;       // HTTPS port
const char WRITE_API_KEY[] = "POWWNFLAIARHZL10";  // Your ThingSpeak write API key
const char READ_API_KEY[] = "43F8VBLWVJP4Y2FN";   // Your ThingSpeak read API key
const char CHANNEL_ID[] = "455094";               // Your ThingSpeak channel ID

// Timeout Configuration (in milliseconds)
const unsigned long CONNECTION_TIMEOUT = 30000;  // Maximum time to establish GSM connection
const unsigned long COMMAND_TIMEOUT = 5000;      // Maximum time for AT commands
const unsigned long DATA_TIMEOUT = 10000;        // Maximum time for data transmission

// ============================================================================
// CLASS DEFINITIONS
// ============================================================================

/**
 * GSM Connection Management Class
 * Handles GSM/GPRS connections with automatic error recovery
 * Features:
 * - Non-blocking connection establishment
 * - Automatic reconnection on failures
 * - Connection state monitoring
 * - Data transmission with error handling
 */
class GSMConnection {
private:
  GSMSSLClient client;           // SSL client for secure connections
  GPRS gprs;                     // GPRS connection handler
  GSM gsmAccess;                 // GSM access handler
  bool isConnected;               // Current connection state
  unsigned long lastConnectionAttempt;  // Timestamp of last connection attempt
  unsigned long connectionTimeout;       // Connection timeout timestamp

public:
  GSMConnection();
  bool connect();                // Establish GSM connection
  bool isReady();                // Check if connection is ready
  void disconnect();              // Disconnect from GSM network
  bool sendData(const char* data);                    // Send data to ThingSpeak
  bool sendData(const char* url, const char* method); // Send HTTP request
  void handleResponse();          // Handle incoming responses
};

/**
 * Non-Blocking Delay Utility Class
 * Replaces blocking delay() calls with state-aware timing
 * Allows other operations to continue while waiting
 */
class NonBlockingDelay {
private:
  unsigned long startTime;        // When the delay started
  unsigned long duration;         // How long to delay (ms)

public:
  NonBlockingDelay(unsigned long ms);  // Constructor with delay duration
  bool isComplete();                   // Check if delay is finished
  void reset(unsigned long ms = 0);   // Reset delay with optional new duration
};

/**
 * Optimized Sensor Data Buffer Class
 * Efficiently stores and manages sensor readings
 * Features:
 * - Fixed-size buffer to prevent memory fragmentation
 * - Automatic data chunking for large datasets
 * - Memory-efficient string conversion
 * - Support for multiple data formats
 */
class SensorBuffer {
private:
  static const int MAX_SAMPLES = 500;  // Maximum number of samples to store
  int samples[MAX_SAMPLES];             // Array to store sensor readings
  int sampleCount;                      // Number of samples currently stored
  int currentIndex;                     // Current write position

public:
  SensorBuffer();
  void addSample(int value);            // Add a new sensor reading
  void clear();                         // Clear all stored data
  int getSampleCount();                 // Get number of stored samples
  bool isFull();                        // Check if buffer is full
  void getDataAsString(char* buffer, int maxLength);           // Convert to string
  void getDataAsChunks(char chunks[][200], int maxChunks, int& chunkCount); // Split into chunks
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/**
 * Wait for specific response from GSM module
 * @param serial - SoftwareSerial object to monitor
 * @param expected - Expected response string
 * @param timeout - Maximum time to wait (ms)
 * @return true if expected response received, false if timeout
 */
bool waitForResponse(SoftwareSerial& serial, const char* expected, unsigned long timeout = COMMAND_TIMEOUT);

/**
 * Send AT command and wait for response
 * @param serial - SoftwareSerial object to use
 * @param command - AT command to send
 * @param expected - Expected response (default: "OK")
 * @param timeout - Maximum time to wait (ms)
 * @return true if command successful, false if failed or timeout
 */
bool sendATCommand(SoftwareSerial& serial, const char* command, const char* expected = "OK", unsigned long timeout = COMMAND_TIMEOUT);

/**
 * Clear all data from serial buffer
 * @param serial - SoftwareSerial object to clear
 */
void clearSerialBuffer(SoftwareSerial& serial);

/**
 * Get amount of free RAM memory
 * @return Number of free bytes
 */
unsigned long getFreeMemory();

/**
 * Print current memory usage to Serial
 * Useful for debugging memory leaks and optimization
 */
void printMemoryUsage();

// ============================================================================
// THINGSPEAK INTEGRATION FUNCTIONS
// ============================================================================

/**
 * Build ThingSpeak URL for data writing or reading
 * @param buffer - Output buffer for the URL
 * @param field - ThingSpeak field number (1-8)
 * @param value - Value to write (for write operations)
 * @param isWrite - true for write URL, false for read URL
 */
void buildThingSpeakURL(char* buffer, int field, int value, bool isWrite = true);

/**
 * Build ThingSpeak URL for reading data
 * @param buffer - Output buffer for the URL
 * @param field - ThingSpeak field number (1-8)
 * @param results - Number of results to retrieve (default: 2)
 */
void buildThingSpeakReadURL(char* buffer, int field, int results = 2);

/**
 * Send data to ThingSpeak cloud platform
 * @param data - Data string to send
 * @param field - ThingSpeak field number (default: 1)
 * @return true if successful, false if failed
 */
bool sendToThingSpeak(const char* data, int field = 1);

#endif
