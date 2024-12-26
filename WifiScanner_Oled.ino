/*
 * WiFi Scanner with OLED Display
 * Created: December 26, 2024
 * Author: Chengmania
 * Description:
 * This program scans for available Wi-Fi networks using an XIAO ESP32S3 and displays the 
 * top three networks with the strongest signal strength on an OLED screen. 
 * The signal strength is also graphically represented as a bar graph.
 * 
 * License: Open Source under the MIT License.
 * You are free to use, modify, and distribute this code with attribution.

Wiring for XIAO ESP32S3 
   

    oLED   SCL  -> XIAO D5
    oLED   SDA  -> XIAO D4
    oLED   GND  -> Ground
    oLED   VCC  -> XIAO 3.3V

 */
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display settings
#define SCREEN_WIDTH 128   // Width of the OLED display in pixels
#define SCREEN_HEIGHT 32   // Height of the OLED display in pixels
#define OLED_RESET    4    // Reset pin (can be -1 if not used)
#define SCREEN_ADDRESS 0x3C // Address for 128x32 OLED

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  // Start the Serial Monitor
  Serial.begin(115200);

  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed")); // Print error if display initialization fails
    for (;;); // Stop execution if the display doesn't initialize
  }

  // Clear the display and print an initialization message
  display.clearDisplay();
  display.setTextSize(1);               // Set text size to 1 (default size)
  display.setTextColor(SSD1306_WHITE); // Set text color to white
  display.setCursor(0, 10);            // Set cursor position
  display.println("WiFi Scanner Init...");
  display.display();                   // Update the display with the message
  delay(1000);

  // Set Wi-Fi to station mode and ensure it's disconnected from any previous network
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
}

// Function to draw a signal strength bar
void drawSignalBar(int x, int y, int rssi) {
  // Normalize RSSI value (-100 dBm to 0 dBm) to fit within a 20-pixel wide bar
  int barLength = map(rssi, -100, 0, 0, 20);
  barLength = constrain(barLength, 0, 20); // Ensure the bar length is between 0 and 20 pixels

  // Draw the bar on the OLED display
  display.fillRect(x, y, barLength, 6, SSD1306_WHITE); // Draw a filled rectangle for the signal bar
}

void loop() {
  Serial.println("Scan start");

  // Scan for Wi-Fi networks
  int n = WiFi.scanNetworks(); // Returns the number of networks found
  Serial.println("Scan done");

  if (n == 0) {
    // No networks found
    Serial.println("No networks found");
    display.clearDisplay();       // Clear the display
    display.setCursor(0, 0);      // Set cursor to the top-left corner
    display.println("No networks found");
    display.display();            // Update the display
  } else {
    Serial.printf("%d networks found\n", n);

    // Store SSIDs and RSSIs in a struct array
    struct WiFiNetwork {
      String ssid; // Network SSID
      int rssi;    // Signal strength (RSSI)
    };
    WiFiNetwork networks[n]; // Array to hold network details
    for (int i = 0; i < n; ++i) {
      networks[i] = { WiFi.SSID(i), WiFi.RSSI(i) }; // Populate the array with network data
    }

    // Sort networks by RSSI in descending order (strongest signal first)
    for (int i = 0; i < n - 1; ++i) {
      for (int j = i + 1; j < n; ++j) {
        if (networks[j].rssi > networks[i].rssi) {
          WiFiNetwork temp = networks[i];
          networks[i] = networks[j];
          networks[j] = temp;
        }
      }
    }

    // Display the top 3 networks on Serial and OLED
    display.clearDisplay(); // Clear the display before updating it

    int displayCount = min(3, n); // Show up to 3 networks
    for (int i = 0; i < displayCount; ++i) {
      int yOffset = 0 + (i * 10); // Vertical spacing for each entry

      // Print SSID
      display.setCursor(0, yOffset); // Set cursor for SSID
      display.printf("%d.%s", i + 1, networks[i].ssid.substring(0, 12).c_str()); // Limit SSID to 12 characters

      // Draw Signal Bar
      drawSignalBar(90, yOffset - 0, networks[i].rssi); // Draw signal strength bar

      // Print RSSI value
      display.setCursor(105, yOffset); // Set cursor for RSSI value
      display.printf("%d", networks[i].rssi); // Print RSSI
    }
    display.display(); // Update the OLED display with new data
  }

  // Free memory used by the scan and wait before scanning again
  WiFi.scanDelete();
  delay(5000); // Wait for 5 seconds before the next scan
}
