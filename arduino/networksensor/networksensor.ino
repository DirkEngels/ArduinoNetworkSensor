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


// Define static
#define DHTTYPE DHT11      // DHT 11
#define DHTPIN A0          // what pin we're connected to
DHT dht(DHTPIN, DHTTYPE);


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
    
  // Setup serial debugging support
  Serial.begin(9600);
  Serial.println("Initializing device");

  // Initalize the Ethernet connection
  Serial.println("Initializing ethernet connection");
  Ethernet.begin(mac, ip);
  
  // Blinking
  Serial.println("Starting blink sequence");
  ledSwitch( 1 );
  delay(500);
  ledSwitch( 0 );
  delay(500);
  ledSwitch( 1 );
  delay(500);
  ledSwitch( 0 );
  
  // Initializing DHT chip
  Serial.println("Initializing DHT chip");
  dht.begin();

  // Start the webserver
  Serial.println("Starting webserver");
  server.begin();
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
  }

  if (client) {
    // Print debug
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
