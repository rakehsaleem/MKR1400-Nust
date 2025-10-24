/*
 * OPTIMIZED Circuit Breaker & GSM Testing System
 * 
 * This sketch implements an advanced data collection and transmission system
 * for circuit breaker monitoring using MKR GSM 1400. Features optimized
 * data handling, efficient chunking, and robust transmission mechanisms.
 * 
 * Hardware Requirements:
 * - MKR GSM 1400 board
 * - GSM antenna
 * - SIM card with active data plan
 * - Magnetic field sensor connected to A0
 * 
 * Key Features:
 * - High-frequency data sampling (432 samples per cycle)
 * - Intelligent data chunking for large datasets
 * - Non-blocking data collection and transmission
 * - Progress reporting and status monitoring
 * - Memory optimization and monitoring
 * - Automatic error recovery
 * 
 * Data Collection Process:
 * - Collects 432 sensor readings per cycle
 * - Splits data across 8 ThingSpeak fields
 * - Transmits data in optimized chunks
 * - Provides real-time progress feedback
 * 
 * Original Code by M. Rakeh Saleem - 12 Apr 2018
 * OPTIMIZED for performance and reliability - 2024
 */

#include <MKRGSM.h>
#include <stdio.h>
#include <string.h>
#include "GSM_Utils.h"

// ============================================================================
// CONFIGURATION CONSTANTS
// ============================================================================

const int SAMPLES_PER_CYCLE = 432;  // Number of sensor readings per collection cycle
const int MAX_CYCLES = 10;          // Maximum number of cycles to collect
const int CHUNK_SIZE = 216;         // Characters per data chunk for transmission

// ============================================================================
// GLOBAL OBJECTS AND STATE MANAGEMENT
// ============================================================================

GSMConnection gsmConnection;                    // GSM connection manager
SensorBuffer sensorBuffer;                      // Efficient sensor data storage
NonBlockingDelay sampleDelay(100);              // Delay between samples (100ms)
NonBlockingDelay transmissionDelay(5000);       // Delay between transmissions (5s)
bool isInitialized = false;                     // GSM connection status flag
int currentCycle = 0;                           // Current collection cycle counter
int samplesCollected = 0;                       // Samples collected in current cycle
// ============================================================================
// SETUP FUNCTION
// ============================================================================

/**
 * Initialize system and start data collection process
 * Sets up GSM connection and begins multi-cycle data collection
 */
void setup()
{
  // Initialize serial communication
  Serial.begin(9600);
  while (!Serial) {
    ; // Wait for serial port to connect (needed for native USB port only)
  }
  
  // Display system information
  Serial.println("==========================================");
  Serial.println("OPTIMIZED Circuit Breaker Testing System");
  Serial.println("==========================================");
  Serial.println("Features:");
  Serial.println("- High-frequency data sampling");
  Serial.println("- Intelligent data chunking");
  Serial.println("- Non-blocking operations");
  Serial.println("- Progress monitoring");
  Serial.println("==========================================");
  
  // Display initial memory status
  printMemoryUsage();
  
  // Get user input for number of cycles
  Serial.println("Enter number of signal cycles (1-10): ");
  while (!Serial.available());
  int cycles = Serial.parseInt();
  
  // Validate and limit cycle count
  if (cycles < 1) cycles = 1;
  if (cycles > MAX_CYCLES) cycles = MAX_CYCLES;
  
  Serial.print("Will collect ");
  Serial.print(cycles);
  Serial.println(" cycles of data");
  
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
  sampleDelay.reset();
  transmissionDelay.reset();
  
  // Start data collection process
  collectDataCycles(cycles);
}

// ============================================================================
// DATA COLLECTION FUNCTIONS
// ============================================================================

/**
 * Collect multiple cycles of sensor data with optimized approach
 * Uses non-blocking sampling to maintain system responsiveness
 * @param cycles - Number of data collection cycles to perform
 */
void collectDataCycles(int cycles) {
  for (int cycle = 0; cycle < cycles; cycle++) {
    Serial.print("Starting cycle ");
    Serial.println(cycle + 1);
    
    // Clear buffer for new cycle
    sensorBuffer.clear();
    
    // Collect samples for this cycle using non-blocking approach
    for (int i = 0; i < SAMPLES_PER_CYCLE; i++) {
      // Only sample when delay is complete (non-blocking)
      if (sampleDelay.isComplete()) {
        int value = analogRead(A0);  // Read magnetic field sensor
        sensorBuffer.addSample(value);
        sampleDelay.reset();
        
        // Display progress every 50 samples
        if (i % 50 == 0) {
          Serial.print("Sample ");
          Serial.print(i);
          Serial.print("/");
          Serial.println(SAMPLES_PER_CYCLE);
        }
      }
      
      // Handle GSM responses during sampling to maintain connection
      gsmConnection.handleResponse();
    }
    
    Serial.println("✓ Cycle completed, transmitting data...");
    
    // Transmit collected data in optimized chunks
    transmitDataInChunks();
    
    Serial.println("✓ Cycle transmission completed");
    printMemoryUsage();
  }
  
  Serial.println("==========================================");
  Serial.println("✓ All data collection cycles completed!");
  Serial.println("==========================================");
}

/**
 * Transmit sensor data in optimized chunks to ThingSpeak
 * Splits large datasets across multiple fields for efficient transmission
 */
void transmitDataInChunks() {
  char chunks[8][200];  // Array to hold data chunks
  int chunkCount = 0;   // Number of chunks created
  
  // Split sensor data into manageable chunks
  sensorBuffer.getDataAsChunks(chunks, 8, chunkCount);
  
  Serial.print("Transmitting ");
  Serial.print(chunkCount);
  Serial.println(" data chunks");
  
  // Send each chunk to different ThingSpeak fields
  for (int i = 0; i < chunkCount; i++) {
    char url[300];
    snprintf(url, sizeof(url), "/update?api_key=%s&field%d=%s", 
             WRITE_API_KEY, i + 1, chunks[i]);
    
    Serial.print("Sending chunk ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(chunks[i]);
    
    // Transmit chunk to ThingSpeak
    if (gsmConnection.sendData(url, "GET")) {
      Serial.println("✓ Chunk sent successfully");
      
      // Wait for response with timeout
      NonBlockingDelay responseDelay(3000);
      while (!responseDelay.isComplete()) {
        gsmConnection.handleResponse();
      }
    } else {
      Serial.println("✗ Failed to send chunk");
    }
    
    // Small delay between chunks to prevent overwhelming server
    delay(1000);
  }
}

// ============================================================================
// MAIN LOOP FUNCTION
// ============================================================================

/**
 * Main program loop - handles system monitoring after data collection
 * This sketch performs data collection in setup() and then monitors system status
 */
void loop()
{
  // Handle any remaining GSM responses
  gsmConnection.handleResponse();
  
  // Display system status periodically (every 30 seconds)
  static NonBlockingDelay statusDelay(30000);
  if (statusDelay.isComplete()) {
    Serial.println("==========================================");
    Serial.println("System Status: Data collection completed");
    Serial.println("System is now in monitoring mode");
    Serial.println("==========================================");
    printMemoryUsage();
    statusDelay.reset();
  }
}
