// Log DS18B20 temperatures to SDcard

// Include the libraries we need
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SPI.h>
#include <SD.h>
#include <avr/pgmspace.h>
#include <Wire.h>  // must be incuded here so that Arduino library object file references work
#include <RtcDS3231.h> //Markuna library 

const int chipSelect = 4;

/* DS18B20 Wiring
    PIN 2 is data pin



*/


// DS3212 CONNECTIONS:
// DS3231 SDA --> SDA
// DS3231 SCL --> SCL
// DS3231 VCC --> 3.3v or 5v
// DS3231 GND --> GND

/* SD card wiring:
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
*/
// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 12

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

RtcDS3231 Rtc;

void setup(void)
{
  // start serial port
  Serial.begin(9600);
  Serial.print("compiled: ");
  Serial.print(__DATE__);
  Serial.println(__TIME__);

  //--------RTC SETUP ------------
  Rtc.Begin();

  // if you are using ESP-01 then uncomment the line below to reset the pins to
  // the available pins for SDA, SCL
  // Wire.begin(0, 2); // due to limited pins, use pin 0 and 2 for SDA, SCL

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);


  if (!Rtc.IsDateTimeValid())
  {
    // Common Cuases:
    //    1) first time you ran and the device wasn't running yet
    //    2) the battery on the device is low or even missing

    Serial.println("RTC lost confidence in the DateTime!");

    // following line sets the RTC to the date & time this sketch was compiled
    // it will also reset the valid flag internally unless the Rtc device is
    // having an issue

    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning())
  {
    Serial.println("RTC was not actively running, starting now");
    Rtc.SetIsRunning(true);
  }

  RtcDateTime now = Rtc.GetDateTime();
  if (now < compiled)
  {
    Serial.println("RTC is older than compile time!  (Updating DateTime)");
    Rtc.SetDateTime(compiled);
  }
  else if (now > compiled)
  {
    Serial.println("RTC is newer than compile time. (this is expected)");
  }
  else if (now == compiled)
  {
    Serial.println("RTC is the same as compile time! (not expected but all is fine)");
  }

  // never assume the Rtc was last configured by you, so
  // just clear them to your needed state
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");



  // Start up the library
  sensors.begin();

  // locate devices on the bus
  //  Serial.print("Locating devices...");
  //  Serial.print("Found ");
  //  Serial.print(sensors.getDeviceCount(), DEC);
  //  Serial.println(" devices.");

  // report parasite power requirements
  //  Serial.print("Parasite power is: ");
  //  if (sensors.isParasitePowerMode()) Serial.println("ON");
  //  else Serial.println("OFF");

  // Assign address manually. The addresses below will beed to be changed
  // to valid device addresses on your bus. Device address can be retrieved
  // by using either oneWire.search(deviceAddress) or individually via
  // sensors.getAddress(deviceAddress, index)
  //insideThermometer = { 0x28, 0x1D, 0x39, 0x31, 0x2, 0x0, 0x0, 0xF0 };
  //outsideThermometer   = { 0x28, 0x3F, 0x1C, 0x31, 0x2, 0x0, 0x0, 0x2 };

  // Search for devices on the bus and assign based on an index. Ideally,
  // you would do this to initially discover addresses on the bus and then
  // use those addresses and manually assign them (see above) once you know
  // the devices on your bus (and assuming they don't change).
  //
  // method 1: by index
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");

  // method 2: search()
  // search() looks for the next device. Returns 1 if a new address has been
  // returned. A zero might mean that the bus is shorted, there are no devices,
  // or you have already retrieved all of them. It might be a good idea to
  // check the CRC to make sure you didn't get garbage. The order is
  // deterministic. You will always get the same devices in the same order
  //
  // Must be called before search()
  //oneWire.reset_search();
  // assigns the first address found to insideThermometer
  //if (!oneWire.search(insideThermometer)) Serial.println("Unable to find address for insideThermometer");
  // assigns the seconds address found to outsideThermometer
  //if (!oneWire.search(outsideThermometer)) Serial.println("Unable to find address for outsideThermometer");

  // show the addresses we found on the bus
  // Serial.print("Device 0 Address: ");
  //printAddress(insideThermometer);
  // Serial.println();

  //// Serial.print("Device 1 Address: ");
  // printAddress(outsideThermometer);
  // Serial.println();

  // set the resolution to 9 bit per device
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

  //Serial.print("Device 0 Resolution: ");
  // Serial.print(sensors.getResolution(insideThermometer), DEC);
  // Serial.println();

  // Serial.print("Device 1 Resolution: ");
  // Serial.print(sensors.getResolution(outsideThermometer), DEC);
  // Serial.println();
}

// function to print a device address
//void printAddress(DeviceAddress deviceAddress)
//{
//  for (uint8_t i = 0; i < 8; i++)
//  {
//    // zero pad the address if necessary
//    if (deviceAddress[i] < 16) Serial.print("0");
//    Serial.print(deviceAddress[i], HEX);
//  }
//}

void AddresstoString(DeviceAddress deviceAddress, char *stringAddress)
{
  sprintf(stringAddress, "%X%X%X%X%X%X%X%X",
          &deviceAddress[0], &deviceAddress[1], &deviceAddress[2], &deviceAddress[3],
          &deviceAddress[4], &deviceAddress[5], &deviceAddress[6], &deviceAddress[7]);
}

// function to print the temperature for a device
void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = sensors.getTempC(deviceAddress);
  // Serial.print("Temp C: ");
  // Serial.print(tempC);
  // Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print the temperature for a device
void TemperaturetoString(DeviceAddress deviceAddress, char *stringTemperature)
{
  float tempC = sensors.getTempC(deviceAddress);
  // Serial.print("Temp C: ");
  // Serial.print(tempC);
  // Serial.print(" Temp F: ");
  Serial.print(DallasTemperature::toFahrenheit(tempC));
}

// function to print a device's resolution
void printResolution(DeviceAddress deviceAddress)
{
  Serial.print("Resolution: ");
  Serial.print(sensors.getResolution(deviceAddress));
  Serial.println();
}

// main function to print information about a device
//void printData(DeviceAddress deviceAddress)
//{
//  printAddress(deviceAddress);
//  Serial.print(",");
//  printTemperature(deviceAddress);
//  Serial.println();
//}

/*
   Main function, calls the temperatures in a loop.
*/
char insideAddress[32];
char outsideAddress[32];
float insideTemperature;
float outsideTemperature;
char datestring[20];

void loop(void)
{

  // call sensors.requestTemperatures() to issue a global temperature
  // request to all devices on the bus


  if (!Rtc.IsDateTimeValid())
  {
    // Common Cuases:
    //    1) the battery on the device is low or even missing and the power line was disconnected
    Serial.println("RTC lost confidence in the DateTime!");
  }


  RtcDateTime now = Rtc.GetDateTime();

  bool waitingForTheFifteen;

  waitingForTheFifteen = true;

  do {
      while (now.Second() != 0) {
           now = Rtc.GetDateTime();
      }
      
    switch (now.Minute()) {
      case 0:
        waitingForTheFifteen = false;
        break;
      case 15:
        waitingForTheFifteen = false;
        break;
      case 30:
        waitingForTheFifteen = false;
        break;
      case 45:
        waitingForTheFifteen = false;
        break;
      default:
        now = Rtc.GetDateTime();
      continue;
    }
  } while (waitingForTheFifteen);

// it now must be 0,15,30, or 45 minutes past the hour

  DateTimetoString(now , datestring);

  sensors.requestTemperatures();


  AddresstoString(insideThermometer, insideAddress);
  Serial.print(datestring);

  Serial.print(",");
  Serial.print(insideAddress);

  Serial.print(",");
  float tempC = sensors.getTempC(outsideThermometer);
  outsideTemperature = DallasTemperature::toFahrenheit(tempC);
  Serial.print(outsideTemperature);

  Serial.print(",");
  AddresstoString(outsideThermometer, outsideAddress);
  Serial.print(outsideAddress);

  Serial.print(",");
  tempC = sensors.getTempC(insideThermometer);
  insideTemperature = DallasTemperature::toFahrenheit(tempC);
  Serial.print(insideTemperature);

  Serial.println();


  File dataFile = SD.open("Temps.csv", FILE_WRITE);

  if (dataFile) {
    dataFile.print(datestring);
    dataFile.print(",");
    dataFile.print(insideAddress);
    dataFile.print(",");
    dataFile.print(insideTemperature);
    dataFile.print(",");
    dataFile.print(outsideAddress);
    dataFile.print(",");
    dataFile.print(outsideTemperature);
    dataFile.println();
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening Temps.csv");
  }


}
#define countof(a) (sizeof(a) / sizeof(a[0]))



void DateTimetoString(const RtcDateTime& dt, char *datestring)
{

  sprintf(datestring,
          "%02u/%02u/%04u %02u:%02u:%02u",
          dt.Month(),
          dt.Day(),
          dt.Year(),
          dt.Hour(),
          dt.Minute(),
          dt.Second() );
}
