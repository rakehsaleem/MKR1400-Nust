# MKR1400-Nust Quick Start Guide

## 🚀 Getting Started

### Hardware Setup
1. **MKR GSM 1400 Board** - Main microcontroller with cellular capability
2. **GSM Antenna** - Required for cellular communication
3. **SIM Card** - Active SIM with data plan (APN: "zonginternet")
4. **Sensors** - Connect to analog pin A0 (light sensor, magnetic field sensor, etc.)

### Software Setup
1. **Install Arduino IDE** (version 1.8.0 or later)
2. **Install MKRGSM Library** via Library Manager
3. **Copy GSM_Utils files** to your Arduino libraries folder
4. **Configure settings** in `GSM_Utils.h`:
   - Update API keys for your ThingSpeak channel
   - Modify phone numbers for SMS/call testing
   - Adjust timeout values if needed

### Quick Test Sequence
1. **Upload Task1-GSM-test.ino** for basic GSM functionality
2. **Open Serial Monitor** (9600 baud)
3. **Send 's'** to test SMS sending
4. **Send 'c'** to test voice calls
5. **Upload Task2-Thingspeak-test.ino** for IoT data transmission

## 📋 Available Sketches

| Sketch | Purpose | Key Features |
|--------|---------|--------------|
| `Task1-GSM-test.ino` | GSM Module Testing | SMS, Voice calls, Non-blocking operations |
| `Task2-Thingspeak-test.ino` | IoT Data Transmission | Continuous data logging, Auto-reconnection |
| `Circuitsbreaker&GSMtesting.ino` | High-Frequency Data Collection | 432 samples/cycle, Data chunking |
| `HTTP-GSM-cloud requests.ino` | Direct HTTP Communication | AT commands, State machine |

## ⚙️ Configuration

### ThingSpeak Setup
1. Create account at [thingspeak.com](https://thingspeak.com)
2. Create new channel
3. Copy Write API Key to `GSM_Utils.h`
4. Configure field mappings in your sketches

### GSM Network Settings
- **APN**: "zonginternet" (for Pakistan networks)
- **Login**: Usually empty
- **Password**: Usually empty
- **PIN**: Configure if SIM requires PIN

## 🔧 Troubleshooting

### Common Issues
- **"GSM module not responding"** → Check wiring and power supply
- **"SIM card not ready"** → Verify SIM insertion and PIN
- **"Network not registered"** → Check signal strength and APN
- **"Connection failed"** → Verify internet connectivity and API keys

### Memory Issues
- Monitor memory usage with `printMemoryUsage()`
- Reduce buffer sizes if needed
- Clear buffers after transmission

## 📊 Performance Monitoring

The optimized code includes built-in monitoring:
- **Memory usage tracking** - Prevents memory leaks
- **Connection status** - Automatic reconnection
- **Progress reporting** - Real-time feedback
- **Error logging** - Detailed error messages

## 🎯 Next Steps

1. **Test basic functionality** with Task1-GSM-test.ino
2. **Set up ThingSpeak channel** for data logging
3. **Deploy sensor monitoring** with Task2-Thingspeak-test.ino
4. **Customize for your application** needs
5. **Monitor performance** and adjust timeouts as needed

## 📞 Support

For technical support or questions:
- Check the detailed `OPTIMIZATION_SUMMARY.md`
- Review inline code comments
- Monitor Serial output for error messages
- Verify hardware connections and network settings

---
*This guide covers the essential steps to get your MKR1400-Nust system running quickly and efficiently.*
