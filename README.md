# infant-car-monitor

This project was submitted to a local science/engineering fair. It's a simplified child monitor that uses an Arduino Uno, Adafruit Wifi Shield, and temperature/motion sensors to detect the movement of a child or pet in a car and then send an SMS message to a preset phone number alerting a parent/pet owner that their car is dangerously hot. 

When the Arduino is booted up, it immediately will use preset wifi credentials to connect to a local wifi network (though this can be modified to work with a cell network). If the internal temperature of the car exceeds a certain limit and motion inside the car (from an infant or pet) is detected, the system sends out an API call to Temboo, an IOT platform. Temboo in turn sends it to the Twilio API, which proceeds to send an alert to a preset phone number. 

It's not a very polished piece of software, but it could be expanded to work with an app or even directly interface with the car to turn up the AC if no one comes to the car. 

CAUTION: This piece of software is unreliable and should never be deployed in the real world. DO NOT LEAVE YOUR CHILD UNATTENDED. Use common sense and don't be an idiot. 
