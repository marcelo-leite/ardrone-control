

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <IPAddress.h>
#include <WiFiUdp.h>

#include "navdata.h"
#define NUM_MAX_ARGS  8
const IPAddress drone(192, 168, 1, 1);


typedef struct {
  String type;
  String cmdargs[NUM_MAX_ARGS];

} ATCommand;



class ArdroneControl{
  unsigned int sequence;          // Current client-side sequence number
  unsigned int lastNav;           // Last sequence number received from AR Drone
  unsigned long lastSend;          // millis since last command packet sent
  unsigned long lastReceive;      // millis since last nav packet received
  WiFiUDP NavUdp;
  WiFiUDP AT;
  String ssid;
//  String drone;
  
  char incoming[4096];
  char block[1024];
  navdata_t navdata;
  uint16_t *id;
  uint16_t *siz;
  int navPort;
  int atPort;
  unsigned long littlecounter;
//  IPAddress drone;

  public:
  
    ArdroneControl(void){
      ssid = "ardrone2_129312"; //1 = 092417, 0 = 116276
//      drone = "192.168.1.1";
      navPort = 5554;
      atPort = 5556;
//      IPAddress drone(192 ,168, 1, 1);
   
      
    }

    void ArdroneConnect(void) {
      // Connect to WiFi network
      Serial.println("connecting to AR WiFi");
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid);
      while (WiFi.status() != WL_CONNECTED) {
        delay(200);
      }
      Serial.print("Connected, address=");
      Serial.println(WiFi.localIP()); 
    
      // Prep UDP
      Serial.println("Starting UDP...");
      NavUdp.begin(navPort); //Open port for navdata
      NavUdp.flush();
      AT.begin(atPort);
      AT.flush();
    
      lastNav = 0;
      sequence = 1;
    
      while(NavUdp.parsePacket() == 0) {
        delay(10);
        NavUdp.beginPacket(drone, navPort);
        NavUdp.write(0x01);
        NavUdp.endPacket();
        delay(20);
        configCommand("general:navdata_demo", "FALSE");
        configCommand("control:altitude_max", "3000");
      }
    
      // Set emergency bit high for 1 second
//      refCommand(false, true);
      unsigned long emergencyInitCounter = millis();
      while (millis() - emergencyInitCounter < 1000) {
        configCommand("general:navdata_demo", "FALSE");
        delay(30);
      }
      littlecounter = millis();
    }
    void serialize(ATCommand command, int Nargs){
      String serialized;
      String argument;
      serialized = "AT*" + (String)command.type + "=" + (String)sequence;
      for(int i =0; i < Nargs; i++){
        argument = "," + (String)command.cmdargs[i];
        serialized.concat(argument);
      }
      ++sequence;
      sendPacket(serialized);
       
    }
    
    void sendPacket(String command) {
      
      char sendChar[command.length() + 1];
      command.toCharArray(sendChar, command.length() + 1);
      sendChar[command.length()] = '\r';
    //  Serial.print("\n");
    //  Serial.print(sendChar);
      AT.beginPacket(drone, atPort);
      AT.write(sendChar);
      AT.endPacket();
      lastSend = millis();
    }
    
    
    bool refCommand(bool takeoff, bool emergency) {
      ATCommand command;
      command.type = "REF";
      if (emergency) {
        command.cmdargs[0] = "290717952";
      } else {
        if (takeoff) {
          command.cmdargs[0] = "290718208";
        } else {
          command.cmdargs[0] = "290717696";
        }
      }
      serialize(command, 1);
//      Serial.print("\n");
      return true;
    }
    
    
    bool pcmdCommand(bool mode, float vel[4]){
      ATCommand command;
      command.type = "PCMD";
      
      command.cmdargs[0] = mode;            //Progressive command or hover command
      
      command.cmdargs[1] = *((int*)&vel[1]);  //Vy
      command.cmdargs[2] = *((int*)&vel[0]);  //Vx
      command.cmdargs[3] = *((int*)&vel[2]);  //Vz
      
      command.cmdargs[4] = *((int*)&vel[3]);  //Wz
      
      serialize(command, 5);
      return true;
    }
    
    bool ftrimCommand(){
      ATCommand command;
      command.type = "FTRIM";
      serialize(command, 0);
      return true;
    }
    
    bool comwdgCommand(){
      ATCommand command;
      command.type = "COMWDG";
      serialize(command, 0);
      return true; 
    }
    
    bool ledCommand(){
      ATCommand command;
      command.type = "LED";
      serialize(command, 4);
      return true; 
    }
    
    bool calibCommand(){
      ATCommand command;
      command.type = "CALIB";
      serialize(command, 2);
      return true; 
    }
    
    bool pwmCommand(int pwm[4]){
      ATCommand command;
    //  [0-500] PWM Interval
      command.type = "PWM";
      command.cmdargs[0] = String(pwm[0]);  //M1
      command.cmdargs[1] = String(pwm[1]);  //M2
      command.cmdargs[2] = String(pwm[2]);  //M3
      command.cmdargs[3] = String(pwm[3]);  //M4
      serialize(command, 4);
      return true;
      
    }
    
    bool configCommand(String key, String value){
      ATCommand command;
      command.type = "CONFIG";
      command.cmdargs[0] = "\"" + String(key) + "\"";
      command.cmdargs[1] = "\"" + String(value) + "\"";
      serialize(command, 2);
    }
    
    
    bool land(){
      return refCommand(false, false);
    }
    
    bool takeoff(){
      return refCommand(true, false);
    }
    
    bool emergency(){
      return refCommand(false, true);
    }

    void arnavdata(){
      
      navdata.head = *((int32_t*)&incoming[0]);
      navdata.ardrone_state = *((int32_t*)&incoming[4]);
      navdata.sequence = *((int32_t*)&incoming[8]);
      navdata.vision = *((int32_t*)&incoming[12]); 
      int len;
      uint16_t k = 16;
      
      if(NavUdp.parsePacket()) {
        lastReceive = millis();
        len = NavUdp.read(incoming, 4096); 
       }
      navdata.id = (uint16_t*)&incoming[16];
      navdata.siz = (uint16_t*)&incoming[18];
      
      for(uint16_t i = 0; i < 31; i++){
        
        for(uint16_t j = 0; j <= *navdata.siz; j++){
          block[j] = incoming[k + j];
          
        }
        navdata.id = (uint16_t*)&block[0];
        navdata.siz = (uint16_t*)&block[2];
        k += *navdata.siz;
    
        switch (*navdata.id){
          
          case 0:
            navdata.block.navdata_demo = *((navdata_demo_t*)&block[4]);
            break;
          case 1:
            navdata.block.navdata_time = *((navdata_time_t*)&block[4]);
            break;
          case 2:
            navdata.block.navdata_raw_measures = *((navdata_raw_measures_t*)&block[4]);
            break;
          case 3:
            navdata.block.navdata_phys_measures = *((navdata_phys_measures_t*)&block[4]);
            break;
          case 4:
            navdata.block.navdata_gyros_offsets = *((navdata_gyros_offsets_t*)&block[4]);
            break;
          case 5:
            navdata.block.navdata_euler_angles = *((navdata_euler_angles_t*)&block[4]);
            break;
          case 6:
            navdata.block.navdata_references = *((navdata_references_t*)&block[4]);
            break;
          case 7:
            navdata.block.navdata_trims = *((navdata_trims_t*)&block[4]);
            break;
          case 8:
            navdata.block.navdata_rc_references = *((navdata_rc_references_t*)&block[4]);
            break;
          case 9:
            navdata.block.navdata_pwm = *((navdata_pwm_t*)&block[4]);
            break;
          case 10:
            navdata.block.navdata_altitude = *((navdata_altitude_t*)&block[4]);
            break;
          case 11:
            navdata.block.navdata_vision_raw = *((navdata_vision_raw_t*)&block[4]);
            break;
          case 12:
            navdata.block.navdata_vision_of = *((navdata_vision_of_t*)&block[4]);
            break;
          case 13:
            navdata.block.navdata_vision = *((navdata_vision_t*)&block[4]);
            break;
          case 14:
            navdata.block.navdata_vision_perf = *((navdata_vision_perf_t*)&block[4]);
            break;
          case 15:
            navdata.block.navdata_trackers_send = *((navdata_trackers_send_t*)&block[4]);
            break;
          case 16:
            navdata.block.navdata_vision_detect = *((navdata_vision_detect_t*)&block[4]);
            break;
          case 17:
            navdata.block.navdata_watchdog = *((navdata_watchdog_t*)&block[4]);
            break;
          case 18:
            navdata.block.navdata_adc_data_frame = *((navdata_adc_data_frame_t*)&block[4]);
            break;
          case 19:
            navdata.block.navdata_video_stream = *((navdata_video_stream_t*)&block[4]);
            break;
          case 20:
            navdata.block.navdata_games = *((navdata_games_t*)&block[4]);
            break;
          case 21:
            navdata.block.navdata_pressure_raw = *((navdata_pressure_raw_t*)&block[4]);
            break;
          case 22:
            navdata.block.navdata_magneto = *((navdata_magneto_t*)&block[4]);
            break;
          case 23:
            navdata.block.navdata_wind_speed = *((navdata_wind_speed_t*)&block[4]);
            break;
          case 24:
            navdata.block.navdata_kalman_pressure = *((navdata_kalman_pressure_t*)&block[4]);
            break;
          case 25:
            navdata.block.navdata_hdvideo_stream = *((navdata_hdvideo_stream_t*)&block[4]);
            break;
          case 26:
            navdata.block.navdata_wifi = *((navdata_wifi_t*)&block[4]);
            break;
          case 27:
            navdata.block.navdata_gps = *((navdata_gps_t*)&block[4]);
            break;
          default:
            break;
        }
      }
      Serial.print("Nivel Bateria:\t" + String(navdata.block.navdata_demo.baterry) + "\n");
      Serial.print("Temperatura:\t" + String(navdata.block.navdata_pressure_raw.temperature_meas) + "\n");
      Serial.print("Pressão:\t" + String(navdata.block.navdata_pressure_raw.pression_meas) + "\n");    
      Serial.print(navdata.block.navdata_trims.euler_angles_trim_phi, 15);
      Serial.print("\n");
      Serial.print(navdata.block.navdata_trims.euler_angles_trim_theta, 15);
      Serial.print("\n");
      Serial.print(navdata.block.navdata_gps.latitude, 8);
      Serial.print("\n");
      Serial.print(navdata.block.navdata_gps.longitude, 8);
      Serial.print("\n");
      Serial.print(navdata.block.navdata_gps.elevation, 8);
      Serial.print("\n");    
      Serial.print("\n\n");
    
      if (millis() - lastSend >= 40) {
        comwdgCommand();
        lastSend = millis();
      }
    
      
    }
    
    
  
};





//ArdroneControl ardrone; 
//
//void setup() {
//  Serial.begin(9600);
//  ardrone.ArdroneConnect();
//  
//}
//
//void loop() {
//  while (WiFi.status() == WL_CONNECTED) {
//
//  }
//    
//  Serial.println("disconnected from AR, attempting to reconnect");
//  ardrone.ArdroneConnect();
//}



//
//ID:   0
//SIZ:  148
//
//ID:   1
//SIZ:  8
//
//ID:   2
//SIZ:  52
//
//ID:   3
//SIZ:  46
//
//ID:   4
//SIZ:  16
//
//ID:   5
//SIZ:  12
//
//ID:   6
//SIZ:  88
//
//ID:   7
//SIZ:  16
//
//ID:   8
//SIZ:  24
//
//ID:   9
//SIZ:  92
//
//ID:   10
//SIZ:  56
//
//ID:   11
//SIZ:  16
//
//ID:   12
//SIZ:  44
//
//ID:   13
//SIZ:  92
//
//ID:   14
//SIZ:  108
//
//ID:   15
//SIZ:  364
//
//ID:   16
//SIZ:  328
//
//ID:   17
//SIZ:  8
//
//ID:   18
//SIZ:  40
//
//ID:   19
//SIZ:  65
//
//ID:   20
//SIZ:  12
//
//ID:   21
//SIZ:  18
//
//ID:   22
//SIZ:  83
//
//ID:   23
//SIZ:  64
//
//ID:   24
//SIZ:  72
//
//ID:   25
//SIZ:  32
//
//ID:   26
//SIZ:  8

//GPS
//ID:   27
//SIZ:  216
//
//ID:   28
//SIZ:  6
//
//ID:   29
//SIZ:  32
//
//ID:   65535
//SIZ:  8
