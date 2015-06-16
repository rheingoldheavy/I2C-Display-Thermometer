/*

 A sketch that displays the temperature on the I2C Display Add-on board, as read
 from an AT30TS750A sensor.

 Datasheet: http://www.atmel.com/images/atmel-8855-dts-at30ts750a-datasheet.pdf

 */

#include <I2C.h>

// Temperature Sensor I2C address and registers
const byte  AT30TS750_I2C           = 0x48;  // Temperature Sensor IC I2C Address
const byte  REGISTER_TEMP           = 0x00;  // Register Address: temperature data
const byte  REG_CONFIG              = 0x01;  // Register Address: Sensor Configuration

// Display Driver I2C address and registers
const byte  AS1115_I2C              = 0x00; // Display Manager IC I2C Address
const byte  REGISTER_SHUTDOWN_MODE  = 0x0C; // Register Address: Display Shutdown
const byte  REGISTER_DECODE         = 0x09; // Register Address: Digit decode / no decode
const byte  REGISTER_SCAN_LIMIT     = 0x0B; // Register Address: Digit enable
const byte  REGISTER_INTENSITY      = 0x0A; // Register Address: Global brightness

const byte  DIGIT_0                 = 0x01; // Left most seven segment digit
const byte  DIGIT_1                 = 0x02; // Second seven segment digit
const byte  DIGIT_2                 = 0x03; // Third seven segment digit
const byte  DIGIT_3                 = 0x04; // Right most seven segment digit
const byte  BTM_RED                 = 0x05; // Bottom Red bar graph multiplexed with L1/L2/L3
const byte  BTM_GRN                 = 0x06; // Bottom Green bar graph
const byte  TOP_RED                 = 0x07; // Top Red bar graph
const byte  TOP_GRN                 = 0x08; // Top Green bar graph

long        errorCount              = 0;    // A variable for holding how many errors occur
byte        errorStatus             = 0;    // A variable to hold the I2C command error code
int         temperature             = 0;


void setup()
{

  pinMode      (A0, OUTPUT);   // Configure the Display multiplexer pin as output
  digitalWrite (A0, HIGH);     // Enable the bottom row red LED, not L1/L2/L3

  I2c.begin    ();             // Initialize the I2C library
  I2c.pullup   (0);            // Disable the internal pullup resistors
  I2c.setSpeed (0);            // Enable 100kHz I2C Bus Speed
  I2c.timeOut  (250);          // Set a 250ms timeout before the bus resets

  Serial.begin(9600);

  init_AT30TS750;
  init_AS1115();
  enableDegreeSymbol (true);
  
}

void loop()
{
  
  // Retrieve the current temperature reading from the AT30TS750
  errorStatus = I2c.read(AT30TS750_I2C, REGISTER_TEMP, 2);
  if (errorStatus != 0) errorHandler();
  temperature = I2c.receive() << 8;
  temperature = temperature | I2c.receive();
  
   // Set digit 0 to the 10's place of the received temperature
  errorStatus = I2c.write(AS1115_I2C, DIGIT_0, (temperature >> 8) / 10);
  if (errorStatus != 0) errorHandler();

  // Set digit 0 to the 1's place of the received temperature
  errorStatus = I2c.write(AS1115_I2C, DIGIT_1, ((temperature >> 8) % 10) | 0x80);
  if (errorStatus != 0) errorHandler();
  
  // Set digit 0 to the 1's place of the received temperature
  errorStatus = I2c.write(AS1115_I2C, DIGIT_2, ((temperature & 0x0080) >> 7) * 5);
  if (errorStatus != 0) errorHandler();

  delay(100);

}

// Sets the degree symbol in the right most digit, DIGIT_3
void enableDegreeSymbol(boolean setSymbol)
{
  if (setSymbol == true)
  {
    // Disable Digit03 Decoding, allow decode of 00-02
    errorStatus = I2c.write(AS1115_I2C, REGISTER_DECODE, 0x07);  
    if (errorStatus != 0) errorHandler();

    // Create degree symbol by setting individual segments
    errorStatus = I2c.write(AS1115_I2C, DIGIT_3, 0x63);     
    if (errorStatus != 0) errorHandler();

  }

  if (setSymbol == false)
  {
    // Enable Digit03 Decoding
    errorStatus = I2c.write(AS1115_I2C, REGISTER_DECODE, 0x0F);  
    if (errorStatus != 0) errorHandler();
    
    // Blank Digit03
    errorStatus = I2c.write(AS1115_I2C, DIGIT_2,         0x0F);          
    if (errorStatus != 0) errorHandler();
  }
}


void init_AT30TS750() {

  /* Change the resolution of the temperature measurement
   0x00 =  9 bit resolution
   0x20 = 10 bit resolution
   0x40 = 11 bit resolution
   0x60 = 12 bit resolution
   */

  errorStatus = I2c.write(AT30TS750_I2C, REG_CONFIG, 0x00);  
  if (errorStatus != 0) errorHandler();
  
}
void init_AS1115()
{ 
  // Enable Digits 0:2
  errorStatus = I2c.write(AS1115_I2C, REGISTER_SCAN_LIMIT,    0x03);  
  if (errorStatus != 0) errorHandler();
  
  // Set digits 0-3 to decode as fonts
  errorStatus = I2c.write(AS1115_I2C, REGISTER_DECODE,        0x0F);  
  if (errorStatus != 0) errorHandler();
  
  // Set the intensity level; 0x00 = LOW, 0x0F = HIGH
  errorStatus = I2c.write(AS1115_I2C, REGISTER_INTENSITY,     0x08);  
  if (errorStatus != 0) errorHandler();
  
  // Begin normal operation
  errorStatus = I2c.write(AS1115_I2C, REGISTER_SHUTDOWN_MODE, 0x01);  
  if (errorStatus != 0) errorHandler();
}


void errorHandler()
{
  
  // We wind up here if an I2C error code was returned.  Increment the error count
  // then display the error code and the current value of errorCount.
  errorCount++;
  Serial.print ("Error Code: 0x");
  Serial.print (errorStatus, HEX);
  Serial.print ("  Error Count Is Now: ");
  Serial.println (errorCount);

  errorStatus = 0;
}
