# Smart Irrigation System with Flood Detection & SMS Alerts  
**By Kabya Ghosh**  
**Date:** 6 June 2022

This Arduino-based smart irrigation system automatically manages water flow using ultrasonic and soil moisture sensors, and communicates via the SIM808 GSM module. It turns on/off pumps based on water level or SMS commands, detects floods, and sends real-time status alerts to your phone.

---

## Features
- **Auto water level control** using ultrasonic distance sensor  
- **SMS-based pump control** (`pump on`, `pump off`, `pump status`)  
- **Flood detection** via soil moisture sensor  
- **Dual relay control** for main and drain pumps  
- **Live status display** on 16x2 I2C LCD  
- **SMS alerts** when water level is reached or flood is detected  
- **Failsafe checks** for proper GSM initialization

---

## Hardware Components
| Component             | Purpose                                 |
|----------------------|-----------------------------------------|
| Arduino UNO           | Main controller                         |
| SIM808 Module         | SMS & GSM communication                 |
| Ultrasonic Sensor     | Water level detection                   |
| Soil Moisture Sensor  | Flood detection                         |
| 2x Relay Module       | Controls main and drain pumps           |
| I2C LCD 16x2 Display  | Real-time system status                 |
| Water Pumps (12V)     | Irrigation and emergency drain          |

---

## Commands via SMS

| SMS Command     | Action                        |
|----------------|-------------------------------|
| `pump on`      | Turns on the main irrigation pump  
| `pump off`     | Turns off the main pump  
| `pump status`  | Sends water level + ON time report

---

## System Behavior
- When **"pump on"** is sent: main pump turns ON and LCD shows live level  
- When **water level drops below 5cm**, the system **auto turns OFF the pump** and sends alert  
- If **flood is detected**, drain pump auto-activates and notifies via SMS  
- All actions are logged on the LCD with clear status messages

---

## Serial Monitor Logs (Optional for Debugging)
- You can read sensor data and state transitions via `Serial.begin(9600)`

---

## Setup and Upload

1. Open the `smart_irrigation.ino` in Arduino IDE  
2. Install required libraries:
   - `DFRobot_SIM808`  
   - `LiquidCrystal_I2C`  
3. Connect components as per pin definitions
4. Upload the code
5. Power up with SIM card inserted and test SMS communication

---

## Pin Mapping

| Function         | Arduino Pin |
|------------------|-------------|
| SIM808 TX/RX     | D10 / D11   |
| Ultrasonic TRIG  | D7          |
| Ultrasonic ECHO  | D8          |
| Main Pump Relay  | D13         |
| Drain Pump Relay | D12         |
| Soil Sensor      | A0          |
| LCD I2C Address  | `0x27` (default) |

---

## Author

**Kabya Ghosh**  
*Student | Robotics & Embedded Systems Enthusiast | CSE @ DIU*  
GitHub: [https://github.com/yourusername](https://github.com/TechByKabya)

---

## License

This project is open-source under the [MIT License](https://opensource.org/licenses/MIT)
