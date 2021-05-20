#include "ardrone-control.cpp"
// #include <SoftwareSerial.h>
// SoftwareSerial RF3DR(D10,D11);
ArdroneControl ardrone; 
ardata_t data;
char buffer[512];
String check = "";
int len = sizeof(ardata_t);

void rfdata();

void setup() {
  Serial.begin(57600);
  
  ardrone.ArdroneConnect();
  
}

void loop() {
  while (WiFi.status() == WL_CONNECTED) {
      rfdata();

      // ardrone.navData();
      // ardrone.VideoStream();
      // data = ardrone.get_ardata();
      // Serial.println(data.fligth_data.nbsat);
      delay(20);
  }
    
  Serial.println("disconnected from AR, attempting to reconnect");
  ardrone.ArdroneConnect();
}



void rfdata(){
    check = "";
    ardrone.navData();
    // ardrone.VideoStream();
    data = ardrone.get_ardata();

    memcpy(&buffer[0], &data, len);
    // Serial.println(sizeof(ardata_t));
    // Serial.println(sizeof(data.pave));
    // Serial.println(data.fligth_data.pression);
    for(int j = 0; j < 7; j++){
      check.concat((char)buffer[j]); 

    }
    
    if( check == "NAVDATA"){
          // Serial.println(check);
      for(int i=0; i < len; i++){
        Serial.write(buffer[i]);
      
        
        
      }

      // Serial.flush();
      Serial.flush();
    }
    
    //  unsigned long millisWithFlushStop = millis();

// Print results for flushed calls
    // Serial.print(F("WITH flush, Serial.println()s return control in: "));
    // Serial.print( millisWithFlushStop - millisWithFlushStart);
    // Serial.println(F(" milliseconds."));

    // Serial.println("\n\n");
    

    // 
    // Serial.println(String(data.signature[0]) + String(data.signature[1]) + String(data.signature[2]) + String(data.signature[3]) + String(data.signature[4])  + String(data.signature[5]) + String(data.signature[6]));
    // Serial.println(data.demo->baterry);
    // Serial.println(data.video_data->signature[0]);
    // Serial.println(sizeof(navdata_block_t));
    // Serial.println(sizeof(navdata_gps_t));
    // Serial.println(sizeof(PaVE_t));
//          RF3DR.flush(); 
//          RF3DR.begin(57600);
//          RF3DR.print("Adrone Dash Board\n");
//          RF3DR.flush();
//          RF3DR.end();
      // Serial.write("Ardrone Dash board\n");
      // Serial.flush();
      
      delay(100);
}