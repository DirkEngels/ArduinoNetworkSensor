/**
 * Arduino Network Sensors
 *
 * This is the arduino source code for the Arduino Network
 * Sensor project.

 * https://github.com/DirkEngels/ArduinoNetworkSensor
 */ 

/**
 * Include Libraries
 */
#include <SPI.h>
#include <Ethernet.h>
#include "DHT.h"
#include <SerialLCD.h>
#include <SoftwareSerial.h>

// Conf DHT
#define DHTTYPE DHT11      // DHT 11
#define DHTPIN A0          // what pin we're connected to
DHT dht(DHTPIN, DHTTYPE);

// Conf  LCD
SerialLCD slcd(3,4);

/**
 * Initialize variables
 */
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
byte ip[] = { 192,168,2,88 };
EthernetServer server(80);

int   stateLed         = 0;
float stateTemperature = 0;
float stateHumidity    = 0;

/**
 * Setup
 * - Set output pins
 * - Load serial support
 */
void setup() {
  pinMode(7, OUTPUT);      // Green light

  // Initializing display & serial debugging
  slcd.begin();
  slcd.backlight();
  slcd.print("Init device");
  Serial.begin(9600);
  Serial.println("Init device");

  // Initalize the Ethernet connection
  slcd.clear();
  slcd.home();
  slcd.print("Init ethernet");
  Serial.println("Initializing ethernet connection");
  Ethernet.begin(mac, ip);
  
  // Blinking
  slcd.clear();
  slcd.home();
  slcd.print("Init LED");
  Serial.println("Starting blink sequence");
  ledSwitch( 1 );
  delay(500);
  ledSwitch( 0 );
  delay(500);
  ledSwitch( 1 );
  delay(500);
  ledSwitch( 0 );
  
  // Initializing DHT chip
  slcd.clear();
  slcd.home();
  slcd.print("Init DHT chip");
  Serial.println("Initializing DHT chip");
  dht.begin();

  // Start the webserver
  slcd.clear();
  slcd.home();
  slcd.print("Init server");
  Serial.println("Starting webserver");
  server.begin();
  
  slcd.clear();
  slcd.home();
  slcd.print("Starting server");
}


/**
 * Loop
 * - Listen for incoming connections
 * - Read (get) request
 * - Switch next led
 */
void loop() {
  // Listen for incoming clients
  EthernetClient client = server.available();
  String requestFirstLine = "";

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    stateTemperature = t;
    stateHumidity    = h;

  // Dislay information on LCD
  slcd.clear();
  slcd.home();

  
  if (stateLed == 1) {
    slcd.print("L:1");
  } else {
    slcd.print("L:0");
  }

  slcd.print(" T:");
  lcdDisplayFloat( stateTemperature ,1);
  slcd.print(stateTemperature);

  slcd.print(" H:");
  lcdDisplayFloat( stateHumidity ,1);
  slcd.setCursor(0,1);
  slcd.print("");
  }

  if (client) {
    // Print debug
    slcd.setCursor(0,1);
    slcd.print("Accepted client");
    Serial.println("Accepted client");

    // An http request ends with a blank line
    boolean currentLineIsBlank = true;
    // Only the first line of the request is needed
    boolean firstLine = true;

    while (client.connected()) {
      if (client.available()) {
        // Append the character to get the uri string
        char c = client.read();
        if ((firstLine) && (c != '\n')) {
          requestFirstLine += c;
        }

        // If you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so the reply can be sent
        if (c == '\n' && currentLineIsBlank) {

          // Switch leds according get request params
          params(requestFirstLine.substring(0,50));

          // Send standard header response
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println();

          // Show output
          output(client);
  
          break;
        }
        if (c == '\n') {
          // New line found
          currentLineIsBlank = true;
          firstLine = false;
        } 
        else if (c != '\r') {
          // Next line contains a character
          currentLineIsBlank = false;
        }
      }
    }

    // Give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    slcd.setCursor(0,1);
    slcd.print("");
    Serial.println("");
  }

}


/**
 * Params
 * - Switch led according params
 */
void params(String requestFirstLine) {
  // Print request params
  Serial.print("- Request data:");
  Serial.println(requestFirstLine);

  // Check get parameters: Led light
  if(requestFirstLine.indexOf("led=on") >0) {
    ledSwitch( 1 );
  }
  if(requestFirstLine.indexOf("led=off") >0) {
    ledSwitch( 0 );
  }
  if(requestFirstLine.indexOf("led=toggle") >0) {
    if( stateLed ) {
      ledSwitch( 0 );
    } else {
      ledSwitch( 1 );
    }
  }
}



/**
 * Switch LED on/off
 */
void ledSwitch( boolean state ) {
  if( state ) {
    digitalWrite(7, HIGH);
    stateLed = 1;
    Serial.println("- Switching red light to ON!");
  } else {
    digitalWrite(7, LOW);
    stateLed = 0;
    Serial.println("- Switching red light to OFF!");
  }
}


void output( EthernetClient client ) {
  // Led
  client.print("Led: ");
  if( stateLed == 1 ) {
    client.print("ON");
  } else {
    client.print("OFF");
  }
  client.println("!");

  // Temperature
  client.print("Temperature: ");
  client.print( stateTemperature );
  client.println(" C");
  
  // Humidity
  client.print("Humidity: ");
  client.print( stateHumidity );
  client.println(" %");
  
}


/**
 * Method copied & renamed from:
 * http://www.seeedstudio.com/wiki/GROVE_-_Starter_Kit_v1.1b
 */
void lcdDisplayFloat(double number, uint8_t digits) { 
  // Handle negative numbers
  if (number < 0.0) {
     slcd.print('-');
     number = -number;
  }

  // Round correctly so that slcd.print(1.999, 2) prints as "2.00"
  double rounding = 0.5;
  for (uint8_t i=0; i<digits; ++i) {
    rounding /= 10.0;
  }
  number += rounding;

  // Extract the integer part of the number and print it
  unsigned long int_part = (unsigned long)number;
  float remainder = number - (float)int_part;
  slcd.print(int_part , DEC); // base DEC

  // Print the decimal point, but only if there are digits beyond
  if (digits > 0)
    slcd.print("."); 

  // Extract digits from the remainder one at a time
  while (digits-- > 0) {
    remainder *= 10.0;
    float toPrint = float(remainder);
    slcd.print(toPrint , DEC);//base DEC
    remainder -= toPrint; 
  }

}
