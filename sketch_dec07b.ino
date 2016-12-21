#include "packetFormat.h"
#include <Wire.h>
// Reference Barometer
#include <Adafruit_BMP085.h>
Adafruit_BMP085 bmp;

// Reference the HMC5883L Compass Library
#include <HMC5883L.h>
// Accelerometer
#include <ADXL345.h>
ADXL345 adxl; //variable adxl is an instance of the ADXL345 library

 // Store our compass as a variable.
 HMC5883L compass;
 // Record any errors that may occur in the compass.
 int error = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin(); // Start the I2C interface.

  //Serial.println("Constructing new HMC5883L");
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  
  //Serial.println("Setting scale to +/- 1.3 Ga");
  error = compass.SetScale(1.30); // Set the scale of the compass.
  if(error != 0) { // If there is an error, print it out.
    //Serial.println("Compass error!");
    //Serial.println(compass.GetErrorText(error));
    delay(1500);
  }
  
  //Serial.println("Setting measurement mode to continous.");
  error = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(error != 0) {// If there is an error, print it out.
    //Serial.println("Compass error!");
    //Serial.println(compass.GetErrorText(error));
    delay(1500);
  }

  
  if (!bmp.begin()) {
    //Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1) {}
  }

  adxl.powerOn();

  //set activity/ inactivity thresholds (0-255)
  adxl.setActivityThreshold(75); //62.5mg per increment
  adxl.setInactivityThreshold(75); //62.5mg per increment
  adxl.setTimeInactivity(10); // how many seconds of no activity is inactive?
 
  //look of activity movement on this axes - 1 == on; 0 == off 
  adxl.setActivityX(1);
  adxl.setActivityY(1);
  adxl.setActivityZ(1);
 
  //look of inactivity movement on this axes - 1 == on; 0 == off
  adxl.setInactivityX(1);
  adxl.setInactivityY(1);
  adxl.setInactivityZ(1);
 
  //look of tap movement on this axes - 1 == on; 0 == off
  adxl.setTapDetectionOnX(0);
  adxl.setTapDetectionOnY(0);
  adxl.setTapDetectionOnZ(1);
 
  //set values for what is a tap, and what is a double tap (0-255)
  adxl.setTapThreshold(50); //62.5mg per increment
  adxl.setTapDuration(15); //625μs per increment
  adxl.setDoubleTapLatency(80); //1.25ms per increment
  adxl.setDoubleTapWindow(200); //1.25ms per increment
 
  //set values for what is considered freefall (0-255)
  adxl.setFreeFallThreshold(7); //(5 - 9) recommended - 62.5mg per increment
  adxl.setFreeFallDuration(45); //(20 - 70) recommended - 5ms per increment
 
  //setting all interupts to take place on int pin 1
  //I had issues with int pin 2, was unable to reset it
  adxl.setInterruptMapping( ADXL345_INT_SINGLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_DOUBLE_TAP_BIT,   ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_FREE_FALL_BIT,    ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_ACTIVITY_BIT,     ADXL345_INT1_PIN );
  adxl.setInterruptMapping( ADXL345_INT_INACTIVITY_BIT,   ADXL345_INT1_PIN );
 
  //register interupt actions - 1 == on; 0 == off  
  adxl.setInterrupt( ADXL345_INT_SINGLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_DOUBLE_TAP_BIT, 1);
  adxl.setInterrupt( ADXL345_INT_FREE_FALL_BIT,  1);
  adxl.setInterrupt( ADXL345_INT_ACTIVITY_BIT,   1);
  adxl.setInterrupt( ADXL345_INT_INACTIVITY_BIT, 1);
}
void loop() {
    Packet data;
    //Serial.print("Temperature = ");
    //Serial.print(bmp.readTemperature());
    data.temperatureCelcius = bmp.readTemperature();
    //Serial.println(" *C");
    //Serial.print("Pressure = ");
    //Serial.print(bmp.readPressure()/133.3);
    data.pressuremmHg = bmp.readPressure()/133.3;
    data.altitude = bmp.readAltitude();
    //Serial.println(" mm");
    //Serial.print("Altitude = ");
    //Serial.print(bmp.readAltitude());
    //Serial.println(" meters");
    //Serial.println();
    data.header = HEADER;
    data.size = sizeof(Packet);
   
    delay(1500);

 
    // Retrive the raw values from the compass (not scaled).
    MagnetometerRaw raw = compass.ReadRawAxis();
    // Retrived the scaled values from the compass (scaled to the configured scale).
    MagnetometerScaled scaled = compass.ReadScaledAxis();
  
    // Values are accessed like so:
    int MilliGauss_OnThe_XAxis = scaled.XAxis;// (or YAxis, or ZAxis)
  
    // Calculate heading when the magnetometer is level, then correct for signs of axis.
    float heading = atan2(scaled.YAxis, scaled.XAxis);
  
    float declinationAngle = 0.0457;
    heading += declinationAngle;
  
    // Correct for when signs are reversed.
    if(heading < 0)
      heading += 2*PI;
  
    // Check for wrap due to addition of declination.
    if(heading > 2*PI)
      heading -= 2*PI;
  
    // Convert radians to degrees for readability.
    float headingDegrees = heading * 180/PI;
  
    // Output the data via the serial port.
    //Output(raw, scaled, heading, headingDegrees);
    data.compassRawX = raw.XAxis;
    data.compassRawY = raw.YAxis;
    data.compassRawZ = raw.ZAxis;
    data.compassScaledX = scaled.XAxis;
    data.compassScaledY = scaled.YAxis;
    data.compassScaledZ = scaled.ZAxis;
    data.heading = headingDegrees;


    //Boring accelerometer stuff   
  int x,y,z;  
  adxl.readAccel(&x, &y, &z); //read the accelerometer values and store them in variables  x,y,z

  // Output x,y,z values - Commented out
  //Serial.println("Accelerometer raw values: ");
 // Serial.print("X axis: "); Serial.print(x); Serial.print(" ");
  //Serial.print("Y axis: "); Serial.print(y); Serial.print(" ");
  //Serial.print("Z axis: "); Serial.println(z); Serial.print(" ");
  data.accelerometerX = x;
  data.accelerometerY = y;
  data.accelerometerZ = z;


  //Fun Stuff!    
  //read interrupts source and look for triggerd actions
  
  //getInterruptSource clears all triggered actions after returning value
  //so do not call again until you need to recheck for triggered actions
   byte interrupts = adxl.getInterruptSource();
  
  // freefall
  if(adxl.triggered(interrupts, ADXL345_FREE_FALL)){
    //Serial.println("freefall");
    strcpy( data.accelerometerDetect, "freefall");
    //add code here to do when freefall is sensed
  } 
  
  //inactivity
  if(adxl.triggered(interrupts, ADXL345_INACTIVITY)){
    //Serial.println("inactivity");
     strcpy( data.accelerometerDetect, "inactivity");
     //add code here to do when inactivity is sensed
  }
  
  //activity
  if(adxl.triggered(interrupts, ADXL345_ACTIVITY)){
    //Serial.println("activity"); 
    strcpy( data.accelerometerDetect,"activity");
     //add code here to do when activity is sensed
  }
  
  //double tap
  if(adxl.triggered(interrupts, ADXL345_DOUBLE_TAP)){
    //Serial.println("double tap");
    strcpy( data.accelerometerDetect,"double tap");
     //add code here to do when a 2X tap is sensed
  }
  
  //tap
  if(adxl.triggered(interrupts, ADXL345_SINGLE_TAP)){
   // Serial.println("tap");
    strcpy( data.accelerometerDetect,"tap");
     //add code here to do when a tap is sensed
  } 
  // write down serial port
   char bites[sizeof(Packet)];
   memcpy(bites, &data, sizeof(Packet));
   Serial.write(bites, sizeof(Packet));
}

// Output the data down the serial port.
void Output(MagnetometerRaw raw, MagnetometerScaled scaled, float heading, float headingDegrees)
{
    Serial.print("Raw:\t");
    Serial.print(raw.XAxis);
    Serial.print("   ");  
    Serial.print(raw.YAxis);
    Serial.print("   ");  
    Serial.print(raw.ZAxis);
    Serial.print("   \tScaled:\t");
    Serial.print(scaled.XAxis);
    Serial.print("   ");  
    Serial.print(scaled.YAxis);
    Serial.print("   ");  
    Serial.print(scaled.ZAxis);
  
    Serial.print("   \tHeading:\t");
    Serial.print(heading);
    Serial.print(" Radians   \t");
    Serial.print(headingDegrees);
    Serial.println(" Degrees   \t");
}

