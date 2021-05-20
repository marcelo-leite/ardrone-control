

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <IPAddress.h>
#include <WiFiUdp.h>


// #include <navdata.h>
// #include <videostream.h>

#include <ardata.h>

#define NUM_MAX_ARGS  8
const IPAddress drone(192, 168, 1, 1);


typedef struct {
  String type;
  String cmdargs[NUM_MAX_ARGS];

} ATCommand;





class ArdroneControl{

  private: 
    unsigned int sequence;          // Current client-side sequence number
    unsigned int lastNav;           // Last sequence number received from AR Drone
    unsigned long lastSend;          // millis since last command packet sent
    unsigned long lastReceive;      // millis since last nav packet received
    WiFiUDP NavUdp;
    WiFiUDP AT;
  //  WiFiUDP Video;
    WiFiClient Video;
    String ssid;
    
    char incoming[4048];
    char block[1024];
    // ARdrone
    navdata_t navdata;
    // fligth_data_t fligth_data;
    // PaVE_t pave;
    // RF Transmission
    
    ardata_t ardata;
    
    
    // ardate.s
    // ardata.navdata_demo = navdata.block.navdata_demo;
    // ardata.video_data = pave;
    
    

    uint16_t *id;
    uint16_t *siz;
    int navPort;
    int atPort;
    int videoPort;
    char stream[128];
  
    unsigned long littlecounter;

  public:
  
    ArdroneControl(void){
      ssid = "ardrone2_129312"; //1 = 092417, 0 = 116276
      navPort = 5554;
      videoPort = 5555;
      atPort = 5556;
      
      strncpy(ardata.signature, "NAVDATA", 7);

      
      // ardata.fligth_data = &fligth_data;
      // ardata.fligth_data->baterry = &navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &navdata.block.navdata_demo.baterry;
      


      // ardata.video_data = &pave; 
    }
    ardata_t get_ardata(){
      return ardata;
    }

    void VideoStream(void){
      String vID;
      // Serial.print("Iniciando Video\n\n");
      // Serial.printf("\n[Connecting to %s ... ", "192.168.1.1");
         if (Video.connect("192.168.1.1", videoPort)){
                Video.readBytes(stream,128);
                // memcpy(&pave, &stream[0], sizeof(PaVE_t));
                
                vID = String(*((char*)&stream[0])) + String(*((char*)&stream[1])) + String(*((char*)&stream[2])) + String(*((char*)&stream[3]));
                if(vID == "PaVE"){
                  memcpy(&ardata.pave, &stream[0], sizeof(PaVE_t));
                
                  // Serial.println(vID);
                  // Serial.println(pave.encoded_stream_width);
                  // Serial.println(pave.encoded_stream_height);
                  // Serial.println(pave.frame_number);
                  // Serial.println(pave.payload_size);
                  // Serial.println(pave.video_codec);
                  // Serial.println(pave.versions);
                  // Serial.println("\n\n");
//                  delay(200);
                }
//            }
          }
       

         




      

      
    }

    
    void ArdroneConnect(void) {
      // Connect to WiFi network
      Serial.println("\n\n\nconnecting to AR WiFi");
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
    
    void configCommand(String key, String value){
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

    void navData(){
//      uint32_t nbsat;
      navdata.head = *((int32_t*)&incoming[0]);
      navdata.ardrone_state = *((int32_t*)&incoming[4]);
      navdata.sequence = *((int32_t*)&incoming[8]);
      navdata.vision = *((int32_t*)&incoming[12]); 
      int len = 0;
      uint16_t k = 16;
      
      if(NavUdp.parsePacket()) {
        lastReceive = millis();
        len = NavUdp.read(incoming, 2048); 
       }
       
      //  else{
      //    while(NavUdp.parsePacket() == 0) {
      //     delay(10);
      //     NavUdp.beginPacket(drone, navPort);
      //     NavUdp.write(0x01);
      //     NavUdp.endPacket();
      //     delay(20);
      //     configCommand("general:navdata_demo", "FALSE");
      //     configCommand("control:altitude_max", "3000");
      //   }
      //   unsigned long emergencyInitCounter = millis();
      //   while (millis() - emergencyInitCounter < 1000) {
      //     configCommand("general:navdata_demo", "FALSE");
      //     delay(30);
      //   }
      //   littlecounter = millis();
      //  }

      navdata.id = (uint16_t*)&incoming[16];
      navdata.siz = (uint16_t*)&incoming[18];
      
      for(uint16_t i = 0; i < 28; i++){
        
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
            // memcpy(&navdata.block.navdata_gps, &block[4], sizeof(navdata_gps_t));
            break;
          default:
            break;
        }
      }

        // ardata.fligth_data.ctrl_state = navdata.block.navdata_demo.ctrl_state;
        // ardata.fligth_data.baterry = navdata.block.navdata_demo.baterry;
        // ardata.fligth_data.theta = navdata.block.navdata_demo.theta;
        // ardata.fligth_data.phi = navdata.block.navdata_demo.psi;
        // ardata.fligth_data.psi = navdata.block.navdata_demo.psi;
        // ardata.fligth_data.altitude = navdata.block.navdata_demo.altitude;
        // ardata.fligth_data.pression = navdata.block.navdata_pressure_raw.pression_meas;
        
        // ardata.fligth_data.v = navdata.block.navdata_demo.v;
        
        // ardata.fligth_data.phys_accs = navdata.block.navdata_phys_measures.phys_accs;
        // ardata.fligth_data.phys_gyros = navdata.block.navdata_phys_measures.phys_gyros;
        
        // ardata.fligth_data.wind_speed = navdata.block.navdata_wind_speed.wind_speed;
        // ardata.fligth_data.wind_angle = navdata.block.navdata_wind_speed.wind_angle;
        
        // ardata.fligth_data.motor[0] = navdata.block.navdata_pwm.motor[0];
        // ardata.fligth_data.motor[1] = navdata.block.navdata_pwm.motor[1];
        // ardata.fligth_data.motor[2] = navdata.block.navdata_pwm.motor[2];
        // ardata.fligth_data.motor[3] = navdata.block.navdata_pwm.motor[3];
        
        // ardata.fligth_data.link_quality = navdata.block.navdata_wifi.link_quality;
        
        // ardata.fligth_data.latitude = navdata.block.navdata_gps.latitude;
        // ardata.fligth_data.longitude = navdata.block.navdata_gps.longitude;
        // ardata.fligth_data.elevation = navdata.block.navdata_gps.elevation;
        // ardata.fligth_data.gps_state = navdata.block.navdata_gps.gps_state;
        // ardata.fligth_data.nbsat = navdata.block.navdata_gps.nbsat;
   
      // ardata->fligth_data.altitude = navdata.block.navdata_demo.altitude;
    
//       Serial.print("Sinal Wifi:\t");
//       Serial.print(navdata.block.navdata_wifi.link_quality);
//       Serial.print("\n");
      
//       Serial.print("Estado:\t");
//       Serial.print(navdata.block.navdata_demo.ctrl_state);
//       Serial.print("\n");

//       Serial.print("Altitude Relativa:\t");
//       Serial.print(navdata.block.navdata_demo.altitude);
//       Serial.print("\n");
      
//       Serial.print("Nivel Bateria:\t");
//       Serial.print(navdata.block.navdata_demo.baterry);
//       Serial.print("\n");
      
//       Serial.print("Pressão:\t");  
//       Serial.print(navdata.block.navdata_pressure_raw.pression_meas);
//       Serial.print("\n")
//       ;  

      Serial.print("Sequencia:\t");
      Serial.print(navdata.sequence);
      Serial.print("\n");

      Serial.print("Latitude:\t");
      Serial.print(navdata.block.navdata_gps.latitude, 8);
      Serial.print("\n");
      
      Serial.print("Longitude:\t");
      Serial.print(navdata.block.navdata_gps.longitude, 8);
      Serial.print("\n");
      
      Serial.print("Elevação:\t");
      Serial.print(navdata.block.navdata_gps.late, 8);
      Serial.print("\n");
      
      Serial.print("Numero de Satelites:\t");
      Serial.print(navdata.block.navdata_gps.nbsat);
      Serial.print("\n");
      Serial.print("\n\n");    
      
//       Serial.print("Quantidade Video:\t");
//       Serial.println(navdata.block.navdata_video_stream.quant);
//       Serial.println(navdata.block.navdata_video_stream.frame_size);
//       Serial.println(navdata.block.navdata_video_stream.frame_number);
//       Serial.println(navdata.block.navdata_video_stream.out_bitrate);
// //      Serial.println(navdata.block.navdata_video_stream.frame_size);
      
      
      
//       Serial.print("\n");

  
      if (millis() - lastSend >= 40) {
        comwdgCommand();
        lastSend = millis();
      }
//      delay(500);
      
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
