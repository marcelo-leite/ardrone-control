#include "ardrone-control.cpp"
// #include <SoftwareSerial.h>
// SoftwareSerial RF3DR(D10,D11);
ArdroneControl ardrone; 
ardata_t data;
char buffer[512];
String check = "";
int len = sizeof(ardata_t);

void rfdata();
void rfCommand();
void comannd_switch(char cmd[]);

void setup() {
  Serial.begin(56700);
  
  ardrone.ArdroneConnect();
  
}

void loop() {
  while (WiFi.status() == WL_CONNECTED) {
      rfdata();
      if(Serial.available()){
        rfCommand();
      }
      // ardrone.navData();
      // ardrone.VideoStream();
      // data = ardrone.get_ardata();
      // Serial.println(data.fligth_data.nbsat);
  //     delay(20);
  }
    
  Serial.println("disconnected from AR, attempting to reconnect");
  ardrone.ArdroneConnect();
}



void rfdata(){


    check = "";
    ardrone.navData();
    // ardrone.videoStream();
    data = ardrone.getArdata();
    if(data.fligth_data.altitude >= 1700){
      ardrone.land();

    }

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
      
      // delay(100);
}



typedef struct{
  
  char ass[3];
  uint8_t size;
  char buffer[40];

} rfdata_t;

void rfCommand(){

    rfdata_t command;
    int i = 0;
    int j = 0;
    int n = 0;
    char temp;
    char size[4];
    char ass[4] = "CMD";

    while (true){
      if(i < 3){
        if(Serial.available()){
          temp = Serial.read();
          if( temp == ass[i] ){
            command.ass[i] = temp;
            i++;
          }else{
            i = 0;
          }
          
        }
        

      }else{
        j = 0;
        while (true){
          if(j < 1){
            if(Serial.available()){
              size[j] = Serial.read();
              j++;
            }
          }else{
            break;
          }
        }

        

        command.size = *((uint8_t*)&size[0]);
        // memcpy(&command.size, &size[0], sizeof(command.size));
        Serial.println(command.size);

        if(command.size >= 4 && command.size <= 30){
          int k = 0;
          while (true){
            if(k < command.size){
              if(Serial.available()){
                command.buffer[k] = Serial.read();
                k++;
              }
            }else{
              comannd_switch(command.buffer);
              break;
            }
          }
          
        }
        
        break;
      }
    }
      
    
}

void comannd_switch(char cmd[]){
    String type;
    float v[4];
    int i = 0;
    for(i = 0; i < 4; i++){
      type.concat(cmd[i]);
    }


    if(type == "TAKE"){
      ardrone.takeoff();
      Serial.println("Decolando");

    }else if(type == "LAND"){
      ardrone.land();
      Serial.println("Aterrissando");

    }else if(type == "PCMD"){
      char vbuffer[16];
      int p;
      for(i = 0; i < 16; i++){
        
        vbuffer[i] = cmd[i + 4];
      }
      // memcpy(&v[0], &vbuffer[0], sizeof(v));
      v[0] = *((float*)&vbuffer[0]);
      v[1] = *((float*)&vbuffer[4]);
      v[2] = *((float*)&vbuffer[8]);
      v[3] = *((float*)&vbuffer[12]);
      // Serial.println(v[0]);
      // Serial.println(v[1]);
      // Serial.println(v[2]);
      // Serial.println(v[3]);
      ardrone.pcmdCommand(true, v);
      
      Serial.println("Controle");
    }

}
