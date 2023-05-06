// External libraries
#include <M5StickCPlus.h>
#include <WiFi.h>

// Project-specific libraries
#include <secrets.h>
#include <tb_display.h>

#define LED_PIN GPIO_NUM_10
#define SCREEN_ORIENTATION 3

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;

int pinState = LOW;

WiFiServer server(80);

//
// Utility
// ----------------------------------------------------------------------------------------
//

void LCD_Reset() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);

  tb_display_init(SCREEN_ORIENTATION);
}

void print(String str) {
  tb_display_print_String(str.c_str(), 50);
  Serial.print(str);
}

void println(String str) {
  tb_display_print_String(str.c_str(), 50);
  tb_display_print_String("\n", 50);
  Serial.println(str);
}

void connectWiFi() {
  print("Connecting to ");
  println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    print(".");
  }
  println("");
  println("WiFi connected.");
  print("IP address: ");
  println(WiFi.localIP().toString());
}

//
// Main
// ---------------------------------------------------------------------------------------
//

void setup() {
  M5.begin();
  LCD_Reset();

  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT); // set the LED pin mode
  pinState = HIGH;
  digitalWrite(LED_PIN, pinState);

  delay(10);

  // We start by connecting to a WiFi network
  connectWiFi();

  server.begin();
}

void loop() {
  WiFiClient client = server.available(); // listen for incoming clients

  if (client) {             // if you get a client,
    println("New Client."); // print a message out the serial port
    String currentLine =
        ""; // make a String to hold incoming data from the client
    while (client.connected()) { // loop while the client's connected
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        if (c == '\n') {         // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a
          // row. that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200
            // OK) and a content-type so the client knows what's coming, then a
            // blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print(
                "Click <a href=\"/high\">here</a> to turn the LED on.<br>");
            client.print(
                "Click <a href=\"/low\">here</a> to turn the LED off.<br>");
            client.print("Click <a href=\"/toggle\">here</a> to toggle the LED "
                         "off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') { // if you got anything else but a carriage
                                // return character,
          currentLine += c;     // add it to the end of the currentLine
        }

        // Routing
        if (currentLine.endsWith("GET /high")) {
          println("LED OFF");
          pinState = HIGH;
        }
        if (currentLine.endsWith("GET /low")) {
          println("LED ON");
          pinState = LOW;
        }
        if (currentLine.endsWith("GET /toggle")) {
          println("LED TOGGLE");
          pinState = 0x1 - pinState;
        }

        digitalWrite(LED_PIN, pinState);
      }
    }
    // close the connection:
    client.stop();
    println("Client Disconnected.");
  }
}
