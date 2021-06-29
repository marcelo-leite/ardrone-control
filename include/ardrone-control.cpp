

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
    
    char incoming[4096];
    char block[2048];
    // ARdrone
    navdata_t navdata;
    // fligth_data_t fligth_data;
    // PaVE_t pave;
    // RF Transmission
    
    ardata_t ardata;
    
    
    // ardate.s
    // ardata.navdata_demo = this->navdata.block.navdata_demo;
    // ardata.video_data = pave;
    
    

    uint16_t *id;
    uint16_t *siz;
    int navPort;
    int atPort;
    int videoPort;
    char stream[10000];
  
    unsigned long littlecounter;

  public:
  
    ArdroneControl(void){
      this->ssid = "ardrone2_129312"; //1 = 092417, 0 = 116276
      this->navPort = 5554;
      this->videoPort = 5555;
      this->atPort = 5556;
      
      strncpy(this->ardata.signature, "NAVDATA", 7);

      
      // ardata.fligth_data = &fligth_data;
      // ardata.fligth_data->baterry = &this->navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &this->navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &this->navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &this->navdata.block.navdata_demo.baterry;
      // ardata.fligth_data->baterry = &this->navdata.block.navdata_demo.baterry;
      


      // ardata.video_data = &pave; 
    }
    ardata_t get_ardata(){
      return this->ardata;
    }

    void VideoStream(void){
      String vID;

      char buffer_payload[6144] = {};
      // Serial.print("Iniciando Video\n\n");
      // Serial.printf("\n[Connecting to %s ... ", "192.168.1.1");
         while (this->Video.connect("192.168.1.1", this->videoPort)){
                // Video.readBytes(stream,5000);
                int i = 0;
                while( i < 10000){
                  if(this->Video.available()){
                    stream[i] = this->Video.read();
                    i++;
                  }

                }
                
                
              
                // memcpy(&pave, &stream[0], sizeof(PaVE_t));
                
                vID = String(*((char*)&this->stream[0])) + String(*((char*)&this->stream[1])) + String(*((char*)&this->stream[2])) + String(*((char*)&this->stream[3]));
                if(vID == "PaVE"){
                  memcpy(&this->ardata.pave, &this->stream[0], sizeof(PaVE_t));
                  // for(uint32_t i =0; i < ardata.pave.payload_size; i++){
                  //   buffer_payload[i] = stream[i + 64];
                  //   // Serial.print(buffer_payload[i]);
                  // }
                  // Serial.println("pronto");
             
                  // Serial.println(vID);
                  Serial.println(this->ardata.pave.encoded_stream_width);
                  Serial.println(this->ardata.pave.encoded_stream_height);
                  // Serial.println(pave.frame_number);
                  Serial.println(this->ardata.pave.payload_size);
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
      WiFi.begin(this->ssid);
      while (WiFi.status() != WL_CONNECTED) {
        delay(200);
      }
      Serial.print("Connected, address=");
      Serial.println(WiFi.localIP()); 
    
      // Prep UDP
      Serial.println("Starting UDP...");
      this->NavUdp.begin(this->navPort); //Open port for navdata
      this->NavUdp.flush();
      this->AT.begin(this->atPort);
      this->AT.flush();
    
      this->lastNav = 0;
      this->sequence = 1;
    
      while(this->NavUdp.parsePacket() == 0) {
        delay(10);
        this->NavUdp.beginPacket(drone, this->navPort);
        this->NavUdp.write(0x01);
        this->NavUdp.endPacket();
        delay(20);
        this->configCommand("general:navdata_demo", "FALSE");
        this->configCommand("control:altitude_max", "3000");
      }
    
      // Set emergency bit high for 1 second
//      refCommand(false, true);
      unsigned long emergencyInitCounter = millis();
      while (millis() - emergencyInitCounter < 1000) {
        this->configCommand("general:navdata_demo", "FALSE");
        delay(30);
      }
      littlecounter = millis();
    }
    
    void serialize(ATCommand command, int Nargs){
      String serialized;
      String argument;
      serialized = "AT*" + (String)command.type + "=" + (String)this->sequence;
      for(int i =0; i < Nargs; i++){
        argument = "," + (String)command.cmdargs[i];
        serialized.concat(argument);
      }
      ++this->sequence;
      this->sendPacket(serialized);
       
    }
    
    void sendPacket(String command) {
      
      char sendChar[command.length() + 1];
      command.toCharArray(sendChar, command.length() + 1);
      sendChar[command.length()] = '\r';
    //  Serial.print("\n");
    //  Serial.print(sendChar);
      this->AT.beginPacket(drone, this->atPort);
      this->AT.write(sendChar);
      this->AT.endPacket();
      this->lastSend = millis();
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
      this->serialize(command, 1);
//      Serial.print("\n");
      return true;
    }
    
    bool pcmdCommand(bool mode, float vel[4]){
      ATCommand command;
      command.type = "PCMD";
      command.cmdargs[0] = mode ? "1" : "0"; // Enables(1) or diables(0) (hover only) movement;            //Progressive command or hover command
      
      command.cmdargs[1] = *((int*)&vel[1]);  //Vy
      command.cmdargs[2] = *((int*)&vel[0]);  //Vx
      command.cmdargs[3] = *((int*)&vel[2]);  //Vz
      
      command.cmdargs[4] = *((int*)&vel[3]);  //Wz
      
      this->serialize(command, 5);
      return true;
    }
    
    bool ftrimCommand(){
      ATCommand command;
      command.type = "FTRIM";
      this->serialize(command, 0);
      return true;
    }
    
    bool comwdgCommand(){
      ATCommand command;
      command.type = "COMWDG";
      this->serialize(command, 0);
      return true; 
    }
    
    bool ledCommand(){
      ATCommand command;
      command.type = "LED";
      this->serialize(command, 4);
      return true; 
    }
    
    bool calibCommand(){
      ATCommand command;
      command.type = "CALIB";
      this->serialize(command, 2);
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
      this->serialize(command, 4);
      return true;
      
    }
    
    void configCommand(String key, String value){
      ATCommand command;
      command.type = "CONFIG";
      command.cmdargs[0] = "\"" + String(key) + "\"";
      command.cmdargs[1] = "\"" + String(value) + "\"";
      this->serialize(command, 2);
    }
    
    bool land(){
      return this->refCommand(false, false);
    }
    
    bool takeoff(){
      return this->refCommand(true, false);
    }
    
    bool emergency(){
      return this->refCommand(false, true);
    }

    void navData(){
//      uint32_t nbsat;
      this->navdata.head = *((int32_t*)&incoming[0]);
      this->navdata.ardrone_state = *((int32_t*)&incoming[4]);
      this->navdata.sequence = *((int32_t*)&incoming[8]);
      this->navdata.vision = *((int32_t*)&incoming[12]); 
      int len = 0;
      uint16_t k = 16;
      
      if(this->NavUdp.parsePacket()) {
        this->lastReceive = millis();
        len = this->NavUdp.read(this->incoming, 4096); 
       }
      this->navdata.id = (uint16_t*)&this->incoming[16];
      this->navdata.siz = (uint16_t*)&this->incoming[18];
      
      for(uint16_t i = 0; i < 28; i++){
        
        for(uint16_t j = 0; j <= *this->navdata.siz; j++){
          this->block[j] = this->incoming[k + j];
          
        }
        this->navdata.id = (uint16_t*)&this->block[0];
        this->navdata.siz = (uint16_t*)&this->block[2];
        k += *this->navdata.siz;
    
        switch (*this->navdata.id){
          
          case 0:
            this->navdata.block.navdata_demo = *((navdata_demo_t*)&this->block[4]);
            break;
          case 1:
            this->navdata.block.navdata_time = *((navdata_time_t*)&this->block[4]);
            break;
          case 2:
            this->navdata.block.navdata_raw_measures = *((navdata_raw_measures_t*)&this->block[4]);
            break;
          case 3:
            this->navdata.block.navdata_phys_measures = *((navdata_phys_measures_t*)&this->block[4]);
            break;
          case 4:
            this->navdata.block.navdata_gyros_offsets = *((navdata_gyros_offsets_t*)&this->block[4]);
            break;
          case 5:
            this->navdata.block.navdata_euler_angles = *((navdata_euler_angles_t*)&this->block[4]);
            break;
          case 6:
            this->navdata.block.navdata_references = *((navdata_references_t*)&this->block[4]);
            break;
          case 7:
            this->navdata.block.navdata_trims = *((navdata_trims_t*)&this->block[4]);
            break;
          case 8:
            this->navdata.block.navdata_rc_references = *((navdata_rc_references_t*)&this->block[4]);
            break;
          case 9:
            this->navdata.block.navdata_pwm = *((navdata_pwm_t*)&this->block[4]);
            break;
          case 10:
            this->navdata.block.navdata_altitude = *((navdata_altitude_t*)&this->block[4]);
            break;
          case 11:
            this->navdata.block.navdata_vision_raw = *((navdata_vision_raw_t*)&this->block[4]);
            break;
          case 12:
            this->navdata.block.navdata_vision_of = *((navdata_vision_of_t*)&this->block[4]);
            break;
          case 13:
            this->navdata.block.navdata_vision = *((navdata_vision_t*)&this->block[4]);
            break;
          case 14:
            this->navdata.block.navdata_vision_perf = *((navdata_vision_perf_t*)&this->block[4]);
            break;
          case 15:
            this->navdata.block.navdata_trackers_send = *((navdata_trackers_send_t*)&this->block[4]);
            break;
          case 16:
            this->navdata.block.navdata_vision_detect = *((navdata_vision_detect_t*)&this->block[4]);
            break;
          case 17:
            this->navdata.block.navdata_watchdog = *((navdata_watchdog_t*)&this->block[4]);
            break;
          case 18:
            this->navdata.block.navdata_adc_data_frame = *((navdata_adc_data_frame_t*)&this->block[4]);
            break;
          case 19:
            this->navdata.block.navdata_video_stream = *((navdata_video_stream_t*)&this->block[4]);
            break;
          case 20:
            this->navdata.block.navdata_games = *((navdata_games_t*)&this->block[4]);
            break;
          case 21:
            this->navdata.block.navdata_pressure_raw = *((navdata_pressure_raw_t*)&this->block[4]);
            break;
          case 22:
            this->navdata.block.navdata_magneto = *((navdata_magneto_t*)&this->block[4]);
            break;
          case 23:
            this->navdata.block.navdata_wind_speed = *((navdata_wind_speed_t*)&this->block[4]);
            break;
          case 24:
            this->navdata.block.navdata_kalman_pressure = *((navdata_kalman_pressure_t*)&this->block[4]);
            break;
          case 25:
            this->navdata.block.navdata_hdvideo_stream = *((navdata_hdvideo_stream_t*)&this->block[4]);
            break;
          case 26:
            this->navdata.block.navdata_wifi = *((navdata_wifi_t*)&this->block[4]);
            break;
          case 27:
            this->navdata.block.navdata_gps = *((navdata_gps_t*)&this->block[4]);
            break;
          default:
            break;
        }
      }
        this->ardata.fligth_data.sequence = this->navdata.sequence;
        this->ardata.fligth_data.adrone_state = this->navdata.ardrone_state;
        this->ardata.fligth_data.baterry = this->navdata.block.navdata_demo.baterry;
        this->ardata.fligth_data.theta = this->navdata.block.navdata_demo.theta;
        this->ardata.fligth_data.phi = this->navdata.block.navdata_demo.psi;
        this->ardata.fligth_data.psi = this->navdata.block.navdata_demo.psi;
        this->ardata.fligth_data.altitude = this->navdata.block.navdata_demo.altitude;
        this->ardata.fligth_data.pression = this->navdata.block.navdata_pressure_raw.pression_meas;
        
        this->ardata.fligth_data.v.x = this->navdata.block.navdata_demo.vx;
        this->ardata.fligth_data.v.y = this->navdata.block.navdata_demo.vy;
        this->ardata.fligth_data.v.z = this->navdata.block.navdata_demo.vz;
        
        this->ardata.fligth_data.phys_accs = this->navdata.block.navdata_phys_measures.phys_accs;
        this->ardata.fligth_data.phys_gyros = this->navdata.block.navdata_phys_measures.phys_gyros;
        
        this->ardata.fligth_data.wind_speed = this->navdata.block.navdata_wind_speed.wind_speed;
        this->ardata.fligth_data.wind_angle = this->navdata.block.navdata_wind_speed.wind_angle;
        
        this->ardata.fligth_data.motor[0] = this->navdata.block.navdata_pwm.motor1;
        this->ardata.fligth_data.motor[1] = this->navdata.block.navdata_pwm.motor2;
        this->ardata.fligth_data.motor[2] = this->navdata.block.navdata_pwm.motor3;
        this->ardata.fligth_data.motor[3] = this->navdata.block.navdata_pwm.motor4;
        
        this->ardata.fligth_data.link_quality = this->navdata.block.navdata_wifi.link_quality;
        
        this->ardata.fligth_data.latitude = this->navdata.block.navdata_gps.latitude;
        this->ardata.fligth_data.longitude = this->navdata.block.navdata_gps.longitude;
        this->ardata.fligth_data.elevation = this->navdata.block.navdata_gps.elevation;
        this->ardata.fligth_data.gps_state = this->navdata.block.navdata_gps.gps_state;
        this->ardata.fligth_data.nbsat = this->navdata.block.navdata_gps.nbsat;
      // ardata->fligth_data.altitude = this->navdata.block.navdata_demo.altitude;
    
//       Serial.print("Sinal Wifi:\t");
//       Serial.print(this->navdata.block.navdata_wifi.link_quality);
//       Serial.print("\n");
      
//       Serial.print("Estado:\t");
//       Serial.print(this->navdata.block.navdata_demo.ctrl_state);
//       Serial.print("\n");

//       Serial.print("Altitude Relativa:\t");
//       Serial.print(this->navdata.block.navdata_demo.altitude);
//       Serial.print("\n");

      // Serial.print("Sequencia:\t");
      // Serial.print(navdata.sequence);
      // Serial.print("\n");
      
      // Serial.print("Nivel Bateria:\t");
      // Serial.print(this->navdata.block.navdata_demo.baterry);
      // Serial.print("\n");
      
      // Serial.print("Pressão:\t");  
      // Serial.print(this->navdata.block.navdata_pressure_raw.pression_meas);
      // Serial.print("\n");  
      // Serial.print("Latitude:\t");
      // Serial.print(this->navdata.block.navdata_gps.latitude, 8);
      // Serial.print("\n");
      
      // Serial.print("Longitude:\t");
      // Serial.print(this->navdata.block.navdata_gps.longitude, 8);
      // Serial.print("\n");
      
      // Serial.print("Elevação:\t");
      // Serial.print(this->navdata.block.navdata_gps.elevation, 8);
      // Serial.print("\n");
      
      // Serial.print("Numero de Satelites:\t");
      // Serial.print(this->navdata.block.navdata_gps.nbsat);
      // Serial.print("\n");
      // Serial.print("\n\n");    
      
//       Serial.print("Quantidade Video:\t");
//       Serial.println(this->navdata.block.navdata_video_stream.quant);
//       Serial.println(this->navdata.block.navdata_video_stream.frame_size);
//       Serial.println(this->navdata.block.navdata_video_stream.frame_number);
//       Serial.println(this->navdata.block.navdata_video_stream.out_bitrate);
// //      Serial.println(this->navdata.block.navdata_video_stream.frame_size);
      
      
      
//       Serial.print("\n");

  
      if (millis() - this->lastSend >= 40) {
        comwdgCommand();
        this->lastSend = millis();
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
