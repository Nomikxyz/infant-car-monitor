#include <OneWire.h> 
#include <SPI.h>
#include "Adafruit_CC3000.h"
#include "Adafruit_CC3000_Server.h"
#include "ccspi.h"
#include <Client.h>
#include <Temboo.h>
#include "TembooAccount.h" // Contains Temboo account information- this is the API that I'm using

#define ADAFRUIT_CC3000_IRQ 3 //adafruit wifi-shield
#define ADAFRUIT_CC3000_VBAT 5
#define ADAFRUIT_CC3000_CS 10

Adafruit_CC3000 cc3k = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);

Adafruit_CC3000_Client client;
int DS18S20_Pin = 2; //DS18S20 Signal pin on digital 2
int sensorValue= 5; //motion sensor

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2



int maxRuns= 11;
int runs= 0;
void setup() {
  Serial.begin(115200);
  pinMode(sensorValue, INPUT);
  // For debugging, wait until the serial console is connected
  delay(4000);
  while(!Serial);

  status_t wifiStatus = STATUS_DISCONNECTED;
  while (wifiStatus != STATUS_CONNECTED) {
    Serial.print("WiFi:");
    if (cc3k.begin()) {
      if (cc3k.connectToAP(WIFI_SSID, WPA_PASSWORD, WLAN_SEC_WPA2)) {
        wifiStatus = cc3k.getStatus();
      }
    }
    if (wifiStatus == STATUS_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
    delay(5000);
  }

  cc3k.checkDHCP();
  delay(1000);

  Serial.println("Setup complete.\n");

  
}



void loop(void) {
 


  float temperature = getTemp();
  Serial.println(temperature); //print the temp
  

  
if (digitalRead(sensorValue) == HIGH && temperature>25 && runs<=maxRuns && temperature!=85) { //if the motion sensor and temperature detect dangerous
                                                             //conditions, trigger a print function and text message

  Serial.println ("Dangerous conditions");
 
    TembooChoreo SendSMSChoreo(client);

    // Invoke the Temboo client
    SendSMSChoreo.begin();

    // Set Temboo account credentials
    SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
    SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set profile to use for execution
    SendSMSChoreo.setProfile("ada");

    // Identify the Choreo to run
    SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

    // Run the Choreo; when results are available, print them to serial
    SendSMSChoreo.run();

    while(SendSMSChoreo.available()) {
      char c = SendSMSChoreo.read();
      Serial.print(c);
    }
    SendSMSChoreo.close();
    Serial.println("\nWaiting...\n");
    runs++;
  delay(1000); // wait 1 second between SendSMS calls

  }

  

else if (digitalRead(sensorValue) && temperature<25) {

  Serial.println ("No dangerous conditions. ");
}

 else if (runs>maxRuns) {
  Serial.println ("Alert! Maximum alerts sent to mobile device.");
 }
 else if (temperature==85) {
  Serial.println ("Electrical fault detected- ignoring this output");
 }
  delay(5000); //just here to slow down the output so it is easier to read

 
  
}


float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  
  return TemperatureSum;
  
}
