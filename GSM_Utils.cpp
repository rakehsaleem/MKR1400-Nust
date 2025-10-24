/*
 * GSM Utilities Library Implementation
 * Optimized common functions for GSM operations
 * 
 * This file contains the implementation of all utility classes and functions
 * defined in GSM_Utils.h. It provides non-blocking GSM operations, efficient
 * data handling, and robust error recovery mechanisms.
 * 
 * Key Features:
 * - Non-blocking GSM connection management
 * - Optimized sensor data buffering
 * - Automatic error recovery and reconnection
 * - Memory monitoring and optimization
 * - ThingSpeak cloud platform integration
 * 
 * Author: Rakeh Saleem
 * Date: 2024
 */

#include "GSM_Utils.h"

// ============================================================================
// GSM CONNECTION CLASS IMPLEMENTATION
// ============================================================================

/**
 * Constructor - Initialize GSM connection object
 * Sets up initial state variables for connection management
 */
GSMConnection::GSMConnection() : isConnected(false), lastConnectionAttempt(0), connectionTimeout(0) {}

/**
 * Establish GSM/GPRS connection with automatic retry logic
 * Uses non-blocking approach to prevent system freezing
 * @return true if connection successful, false if failed
 */
bool GSMConnection::connect() {
  // Check if already connected
  if (isConnected) return true;
  
  unsigned long currentTime = millis();
  
  // Prevent connection attempts too frequently (rate limiting)
  if (currentTime - lastConnectionAttempt < 5000) {
    return false;
  }
  
  lastConnectionAttempt = currentTime;
  connectionTimeout = currentTime + CONNECTION_TIMEOUT;
  
  Serial.println("Attempting GSM connection...");
  
  // Attempt GSM connection
  if (gsmAccess.begin(PINNUMBER) == GSM_READY) {
    // Attempt GPRS connection
    if (gprs.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY) {
      isConnected = true;
      Serial.println("GSM connection established successfully");
      return true;
    }
  }
  
  Serial.println("GSM connection failed - will retry later");
  return false;
}

/**
 * Check if GSM connection is ready and not timed out
 * @return true if connection is active and ready
 */
bool GSMConnection::isReady() {
  return isConnected && (millis() < connectionTimeout);
}

/**
 * Disconnect from GSM network and clean up resources
 * Should be called when shutting down or switching networks
 */
void GSMConnection::disconnect() {
  if (isConnected) {
    client.stop();
    isConnected = false;
    Serial.println("GSM connection closed");
  }
}

/**
 * Send data to ThingSpeak using predefined URL
 * @param url - Complete URL path for ThingSpeak API
 * @param method - HTTP method (GET, POST, PUT)
 * @return true if data sent successfully
 */
bool GSMConnection::sendData(const char* url, const char* method) {
  // Ensure connection is ready
  if (!isReady()) {
    if (!connect()) return false;
  }
  
  // Attempt to connect to ThingSpeak server
  if (client.connect(THINGSPEAK_SERVER, THINGSPEAK_PORT)) {
    // Send HTTP request
    client.print(method);
    client.print(" ");
    client.print(url);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(THINGSPEAK_SERVER);
    client.println("Connection: close");
    client.println();
    return true;
  }
  
  Serial.println("Failed to connect to ThingSpeak server");
  return false;
}

/**
 * Handle incoming responses from server
 * Processes data and manages connection state
 * Should be called regularly in main loop
 */
void GSMConnection::handleResponse() {
  // Process incoming data
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  
  // Check for disconnection
  if (!client.available() && !client.connected()) {
    Serial.println("\nServer disconnected");
    client.stop();
  }
}

// ============================================================================
// NON-BLOCKING DELAY CLASS IMPLEMENTATION
// ============================================================================

/**
 * Constructor - Initialize delay with specified duration
 * @param ms - Delay duration in milliseconds
 */
NonBlockingDelay::NonBlockingDelay(unsigned long ms) : duration(ms) {
  startTime = millis();
}

/**
 * Check if the delay period has completed
 * @return true if delay is finished, false if still waiting
 */
bool NonBlockingDelay::isComplete() {
  return (millis() - startTime) >= duration;
}

/**
 * Reset delay timer with optional new duration
 * @param ms - New delay duration (0 = use current duration)
 */
void NonBlockingDelay::reset(unsigned long ms) {
  startTime = millis();
  if (ms > 0) duration = ms;
}

// ============================================================================
// SENSOR BUFFER CLASS IMPLEMENTATION
// ============================================================================

/**
 * Constructor - Initialize sensor buffer
 * Sets up empty buffer with proper initial state
 */
SensorBuffer::SensorBuffer() : sampleCount(0), currentIndex(0) {
  clear();
}

/**
 * Add new sensor reading to buffer
 * Automatically handles buffer overflow by overwriting oldest data
 * @param value - Sensor reading value to store
 */
void SensorBuffer::addSample(int value) {
  if (currentIndex < MAX_SAMPLES) {
    samples[currentIndex] = value;
    currentIndex++;
    sampleCount = currentIndex;
  }
}

/**
 * Clear all stored sensor data
 * Resets buffer to empty state
 */
void SensorBuffer::clear() {
  currentIndex = 0;
  sampleCount = 0;
  memset(samples, 0, sizeof(samples));
}

/**
 * Get number of samples currently stored
 * @return Number of samples in buffer
 */
int SensorBuffer::getSampleCount() {
  return sampleCount;
}

/**
 * Check if buffer is at maximum capacity
 * @return true if buffer is full
 */
bool SensorBuffer::isFull() {
  return currentIndex >= MAX_SAMPLES;
}

/**
 * Convert stored sensor data to string format
 * Creates space-separated string of all sensor values
 * @param buffer - Output buffer for string data
 * @param maxLength - Maximum length of output buffer
 */
void SensorBuffer::getDataAsString(char* buffer, int maxLength) {
  int pos = 0;
  for (int i = 0; i < sampleCount && pos < maxLength - 10; i++) {
    int written = snprintf(buffer + pos, maxLength - pos, "%d ", samples[i]);
    if (written > 0) pos += written;
  }
  buffer[pos] = '\0';
}

/**
 * Split sensor data into multiple chunks for transmission
 * Useful for sending large datasets that exceed single field limits
 * @param chunks - Output array for data chunks
 * @param maxChunks - Maximum number of chunks to create
 * @param chunkCount - Output parameter for actual number of chunks created
 */
void SensorBuffer::getDataAsChunks(char chunks[][200], int maxChunks, int& chunkCount) {
  chunkCount = 0;
  int samplesPerChunk = sampleCount / maxChunks;
  if (samplesPerChunk == 0) samplesPerChunk = 1;
  
  for (int chunk = 0; chunk < maxChunks && chunkCount < maxChunks; chunk++) {
    int pos = 0;
    int startSample = chunk * samplesPerChunk;
    int endSample = min(startSample + samplesPerChunk, sampleCount);
    
    for (int i = startSample; i < endSample && pos < 190; i++) {
      int written = snprintf(chunks[chunk] + pos, 200 - pos, "%d ", samples[i]);
      if (written > 0) pos += written;
    }
    
    if (pos > 0) {
      chunks[chunk][pos] = '\0';
      chunkCount++;
    }
  }
}

// ============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// ============================================================================

/**
 * Wait for specific response from GSM module with timeout
 * Non-blocking function that checks for expected response
 * @param serial - SoftwareSerial object to monitor
 * @param expected - Expected response string to wait for
 * @param timeout - Maximum time to wait in milliseconds
 * @return true if expected response received, false if timeout
 */
bool waitForResponse(SoftwareSerial& serial, const char* expected, unsigned long timeout) {
  NonBlockingDelay delay(timeout);
  String response = "";
  
  while (!delay.isComplete()) {
    if (serial.available()) {
      char c = serial.read();
      response += c;
      
      // Check if we received the expected response
      if (response.indexOf(expected) >= 0) {
        return true;
      }
    }
  }
  
  return false;
}

/**
 * Send AT command to GSM module and wait for response
 * Provides reliable command execution with timeout handling
 * @param serial - SoftwareSerial object to use
 * @param command - AT command to send
 * @param expected - Expected response (default: "OK")
 * @param timeout - Maximum time to wait for response
 * @return true if command successful, false if failed or timeout
 */
bool sendATCommand(SoftwareSerial& serial, const char* command, const char* expected, unsigned long timeout) {
  serial.println(command);
  return waitForResponse(serial, expected, timeout);
}

/**
 * Clear all data from serial buffer
 * Prevents buffer overflow and ensures clean communication
 * @param serial - SoftwareSerial object to clear
 */
void clearSerialBuffer(SoftwareSerial& serial) {
  while (serial.available()) {
    serial.read();
  }
}

/**
 * Calculate amount of free RAM memory
 * Useful for monitoring memory usage and detecting leaks
 * @return Number of free bytes in RAM
 */
unsigned long getFreeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(malloc(4));
}

/**
 * Print current memory usage information to Serial
 * Displays free memory for debugging and optimization
 */
void printMemoryUsage() {
  Serial.print("Free memory: ");
  Serial.print(getFreeMemory());
  Serial.println(" bytes");
}

// ============================================================================
// THINGSPEAK INTEGRATION FUNCTIONS
// ============================================================================

/**
 * Build ThingSpeak URL for data writing or reading
 * Creates properly formatted URLs for ThingSpeak API calls
 * @param buffer - Output buffer for the URL
 * @param field - ThingSpeak field number (1-8)
 * @param value - Value to write (for write operations)
 * @param isWrite - true for write URL, false for read URL
 */
void buildThingSpeakURL(char* buffer, int field, int value, bool isWrite) {
  if (isWrite) {
    snprintf(buffer, 200, "/update?api_key=%s&field%d=%d", WRITE_API_KEY, field, value);
  } else {
    snprintf(buffer, 200, "/channels/%s/fields/%d/last.json?api_key=%s&results=2", 
             CHANNEL_ID, field, READ_API_KEY);
  }
}

/**
 * Build ThingSpeak URL for reading data from specific field
 * Creates URL for retrieving historical data from ThingSpeak
 * @param buffer - Output buffer for the URL
 * @param field - ThingSpeak field number (1-8)
 * @param results - Number of results to retrieve (default: 2)
 */
void buildThingSpeakReadURL(char* buffer, int field, int results) {
  snprintf(buffer, 200, "/channels/%s/fields/%d/last.json?api_key=%s&results=%d", 
           CHANNEL_ID, field, READ_API_KEY, results);
}

/**
 * Send data to ThingSpeak cloud platform
 * High-level function for easy data transmission
 * @param data - Data string to send
 * @param field - ThingSpeak field number (default: 1)
 * @return true if successful, false if failed
 */
bool sendToThingSpeak(const char* data, int field) {
  GSMConnection gsm;
  char url[300];
  
  // Build URL with data
  snprintf(url, sizeof(url), "/update?api_key=%s&field%d=%s", WRITE_API_KEY, field, data);
  
  // Send data
  if (gsm.sendData(url, "GET")) {
    // Wait for response with timeout
    NonBlockingDelay responseDelay(5000);
    while (!responseDelay.isComplete()) {
      gsm.handleResponse();
      if (!gsm.isReady()) break;
    }
    return true;
  }
  
  return false;
}
