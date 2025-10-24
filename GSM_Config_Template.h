/*
 * GSM Configuration Template
 * Copy this file to GSM_Config.h and customize for your setup
 */

#ifndef GSM_CONFIG_H
#define GSM_CONFIG_H

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================

// SIM Card Configuration
const char PINNUMBER[] = "";            // SIM PIN (empty if not required)

// GPRS/APN Configuration (modify for your network provider)
const char GPRS_APN[] = "zonginternet"; // Change to your provider's APN
const char GPRS_LOGIN[] = "";           // Usually empty
const char GPRS_PASSWORD[] = "";        // Usually empty

// Common APN Settings by Country/Provider:
// Pakistan: "zonginternet", "ufoneinternet", "jazzinternet"
// USA: "att.m2m", "verizon", "tmobile"
// Europe: "internet", "web", "gprs"

// ============================================================================
// THINGSPEAK CONFIGURATION
// ============================================================================

// ThingSpeak Cloud Platform Settings
const char THINGSPEAK_SERVER[] = "api.thingspeak.com";
const int THINGSPEAK_PORT = 443;       // HTTPS port

// Your ThingSpeak Channel Settings (replace with your values)
const char WRITE_API_KEY[] = "YOUR_WRITE_API_KEY_HERE";
const char READ_API_KEY[] = "YOUR_READ_API_KEY_HERE";
const char CHANNEL_ID[] = "YOUR_CHANNEL_ID_HERE";

// Field Mappings (customize based on your sensors)
#define FIELD_LIGHT_SENSOR 1
#define FIELD_TEMPERATURE 2
#define FIELD_HUMIDITY 3
#define FIELD_PRESSURE 4
#define FIELD_MAGNETIC_FIELD 5
#define FIELD_VOLTAGE 6
#define FIELD_GPS_LATITUDE 7
#define FIELD_GPS_LONGITUDE 8

// ============================================================================
// TIMEOUT CONFIGURATION
// ============================================================================

// Timeout Settings (adjust based on network conditions)
const unsigned long CONNECTION_TIMEOUT = 30000;  // GSM connection timeout (ms)
const unsigned long COMMAND_TIMEOUT = 5000;      // AT command timeout (ms)
const unsigned long DATA_TIMEOUT = 10000;        // Data transmission timeout (ms)

// Sampling and Transmission Intervals
const unsigned long SAMPLE_INTERVAL = 5000;     // Data collection interval (ms)
const unsigned long TRANSMIT_INTERVAL = 30000;   // Data transmission interval (ms)

// ============================================================================
// PHONE NUMBERS FOR TESTING
// ============================================================================

// Phone numbers for SMS and call testing (replace with your numbers)
const char PHONE_NUMBER_SMS[] = "+1234567890";   // SMS recipient
const char PHONE_NUMBER_CALL[] = "+1234567890";  // Call destination

// ============================================================================
// DEBUG CONFIGURATION
// ============================================================================

// Debug Settings
#define DEBUG_MODE true              // Enable/disable debug output
#define MEMORY_MONITORING true       // Enable memory usage monitoring
#define VERBOSE_LOGGING false        // Enable detailed logging

// Serial Communication Settings
#define SERIAL_BAUD_RATE 9600        // Serial monitor baud rate
#define GSM_BAUD_RATE 9600           // GSM module baud rate

// ============================================================================
// HARDWARE PIN CONFIGURATION
// ============================================================================

// GSM Module Pins (SoftwareSerial)
#define GSM_RX_PIN 7
#define GSM_TX_PIN 8

// Sensor Pins
#define SENSOR_PIN A0                // Primary sensor input
#define LED_PIN 13                   // Status LED

// ============================================================================
// DATA COLLECTION SETTINGS
// ============================================================================

// Sensor Buffer Configuration
#define MAX_SAMPLES 500              // Maximum samples in buffer
#define SAMPLES_PER_CYCLE 432        // Samples per collection cycle
#define CHUNK_SIZE 216               // Characters per data chunk

// ============================================================================
// ADVANCED CONFIGURATION
// ============================================================================

// Retry Settings
#define MAX_RETRY_ATTEMPTS 3         // Maximum retry attempts
#define RETRY_DELAY 5000             // Delay between retries (ms)

// Power Management
#define LOW_POWER_MODE false         // Enable low power mode
#define SLEEP_DURATION 60000         // Sleep duration in low power mode (ms)

// Data Compression
#define ENABLE_COMPRESSION false     // Enable data compression
#define COMPRESSION_THRESHOLD 1000   // Minimum data size for compression

#endif
