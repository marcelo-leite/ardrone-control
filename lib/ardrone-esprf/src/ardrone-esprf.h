#ifndef ARDRONE_ESPRF_H
#define ARDRONE_ESPRF_H

#include <ardrone_esp.h>
#include <PID.cpp>

typedef struct{
  
  char ass[3];
  uint8_t size;
  char buffer[40];

} rfdata_t;

class ArdroneESPRF{
  public:
    ArdroneESP ardrone;
    PIDControl pid_wz; 
    PIDControl pid_vz;
    ardata_t data;
    char buffer[512];
    String check;
    int len;

    ArdroneESPRF(void);
    
    void captureData(void);
    void sendData(void);
    void receiveData(void);
    void commandSwitch(char cmd[]);
    void control_wz(float setpoint);
    void secureHealth(void);
    
};

#endif  //ARDRONE_ESPRF_H