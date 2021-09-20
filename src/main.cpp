#include <ardrone-esprf.h>

ArdroneESPRF rfcontrol;
void setup() {
  Serial.begin(56700);
  rfcontrol.ardrone.ardroneConnect();
  
}

void loop() {
  while (WiFi.status() == WL_CONNECTED) {
      // 
      // rfcontrol.ardrone.takeoff();
      // delay(10000);
      // rfcontrol.ardrone.takeoff();
      // rfcontrol.ardrone.emergency();
      while (WiFi.status() == WL_CONNECTED) {
        // rfcontrol.captureData();
        rfcontrol.sendData();
        if(Serial.available()){
          rfcontrol.receiveData();
        }
        // rfcontrol.secureHealth();
        // if(rfcontrol.data.fligth_data.adrone_state & 1){
        //   // rfcontrol.control_wz(60.0);
        //   Serial.print("Voando");

        // }else{
        //   Serial.print("Aterrissado");
        // }
        
        // if(Serial.available()){
        //   rfcontrol.receiveData();
        // }
        // rfcontrol.control_wz(120.0);
        
        // Serial.println(rfcontrol.ardrone.getArdata().fligth_data.psi);
        delay(100);
      }
  }
 
  Serial.println("disconnected from AR, attempting to reconnect");
  rfcontrol.ardrone.ardroneConnect();

  // if(Serial.available()){
  //   rfcontrol.receiveData();
  // }
  // delay(100);
}
