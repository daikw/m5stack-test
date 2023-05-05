#include <M5StickCPlus.h>
#include <WiFi.h>

#include "secrets.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASS;

WiFiServer server(80);

void LCD_Reset()
{
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setRotation(1);
  M5.Lcd.setTextSize(1);
}

void write(int n)
{
  M5.Lcd.write(n);
  Serial.write(n);
}

void print(String str)
{
  M5.Lcd.print(str);
  Serial.print(str);
}

void println(String str)
{
  M5.Lcd.println(str);
  Serial.println(str);
}

void connectWiFi()
{
  print("Connecting to ");
  println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    print(".");
  }
  println("");
  println("WiFi connected.");
  println("IP address: ");
  println(WiFi.localIP().toString());
}

void setup()
{
  M5.begin();
  LCD_Reset();

  Serial.begin(115200);
  pinMode(5, OUTPUT); // set the LED pin mode

  delay(10);

  // We start by connecting to a WiFi network
  connectWiFi();

  server.begin();
}

void loop()
{
  WiFiClient client = server.available(); // listen for incoming clients

  if (client)
  {                          // if you get a client,
    println("New Client.");  // print a message out the serial port
    String currentLine = ""; // make a String to hold incoming data from the client
    while (client.connected())
    { // loop while the client's connected
      if (client.available())
      {                         // if there's bytes to read from the client,
        char c = client.read(); // read a byte, then
        write(c);               // print it out the serial monitor
        if (c == '\n')
        { // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0)
          {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          }
          else
          { // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }
        else if (c != '\r')
        {                   // if you got anything else but a carriage return character,
          currentLine += c; // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H"))
        {
          digitalWrite(5, HIGH); // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L"))
        {
          digitalWrite(5, LOW); // GET /L turns the LED off
        }
      }
    }
    // close the connection:
    client.stop();
    println("Client Disconnected.");
  }
}
