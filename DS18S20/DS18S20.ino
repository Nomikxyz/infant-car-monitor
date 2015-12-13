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

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  // on digital pin 2


int timeelapsed= 120000; //this is the number (in milliseconds) that will elapse before another message is sent. 
int maximumruns= 5;
void setup() {
  Serial.begin(9600);
  
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
  int sensorValue = digitalRead(7); //the motion sensor
  
if (sensorValue == HIGH && temperature>23) { //if the motion sensor and temperature detect dangerous
                                             //conditions, trigger a print function and text message

  Serial.println ("Dangerous conditions");
 TembooChoreo SendSMSChoreo(client);

    // Invoke the Temboo client
    SendSMSChoreo.begin();

    // Set Temboo account credentials
    SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
    SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

    // Set Choreo inputs
    String AuthTokenValue = "e5e2a6dcb56e7d7b44a5d2ee27a10c70";
    SendSMSChoreo.addInput("AuthToken", AuthTokenValue);
    String BodyValue = "ALERT- Dangerous conditions detected.";
    SendSMSChoreo.addInput("Body", BodyValue);
    String ToValue = "+12017465282";
    SendSMSChoreo.addInput("To", ToValue);
    String AccountSIDValue = "AC31fb1c701bc8070a9995d9db7b5de8ef";
    SendSMSChoreo.addInput("AccountSID", AccountSIDValue);
    String FromValue = "+17472322264";
    SendSMSChoreo.addInput("From", FromValue);

    // Identify the Choreo to run
    SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

    // Run the Choreo; when results are available, print them to serial
    SendSMSChoreo.run();

    while(SendSMSChoreo.available()) {
      char c = SendSMSChoreo.read();
      Serial.print(c);
    }
    SendSMSChoreo.close();
  }

  Serial.println("\nWaiting...\n");
  delay(30000); // wait 30 seconds between SendSMS calls

}
else if (sensorValue==LOW && temperature<23) {

  Serial.println ("No dangerous conditions. ");
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
