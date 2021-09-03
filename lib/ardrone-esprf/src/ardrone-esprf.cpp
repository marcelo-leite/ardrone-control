#include "ardrone-esprf.h"

//ArdroneESPRF Contructor
ArdroneESPRF::ArdroneESPRF(void){
    this->check = "";
    this->len = sizeof(ardata_t);
    
    // PID WZ 
    this->pid_wz.setLimits(-0.5,0.5);
    this->pid_wz.setTunings(1,0.15,0);

    // PID VZ
}

//Function Member
void ArdroneESPRF::captureData(void){
    this->ardrone.captureNavdata();
    this->data = this->ardrone.getArdata();
    this->ardrone.showNavdata();
}

//Function Member
void ArdroneESPRF::sendData(void){
    this->check = "";
    this->ardrone.captureNavdata();
    // this->ardrone.videoStream();
    this->data = this->ardrone.getArdata();
    memcpy(&this->buffer[0], &this->data, sizeof(data));

    for(int j = 0; j < 7; j++){
        check.concat((char)this->buffer[j]);
    }

    if(this->check == "NAVDATA"){
        for(int i=0; i < len; i++){
            Serial.write(buffer[i]);       
        }
        Serial.flush();
    }
}

//Function Member
void ArdroneESPRF::receiveData(void){
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
            // Serial.println(command.size);

            if(command.size >= 4 && command.size <= 30){
                int k = 0;
                while (true){
                    if(k < command.size){
                        if(Serial.available()){
                            command.buffer[k] = Serial.read();
                            k++;
                        }
                    }else{
                        this->commandSwitch(command.buffer);
                        break;
                    }
                } 
            }
            break;
        }
    }
}

void ArdroneESPRF::commandSwitch(char cmd[]){
    String type;
    float v[4];
    int i = 0;
    for(i = 0; i < 4; i++){
        type.concat(cmd[i]);
    }

    // MACHINE STATE
    if(type == "TAKE"){
        // Decolando
        ardrone.takeoff();
        Serial.println("Decolando");

    }else if(type == "LAND"){
        // Aterrissando
        ardrone.land();
        Serial.println("Aterrissando");

    }else if(type == "PCMD"){
        // Movement Control
        char vbuffer[16];
        int p;
        bool move, hover[4];
        move = true;

        for(i = 0; i < 16; i++){
            
            vbuffer[i] = cmd[i + 4];
        }
        // memcpy(&v[0], &vbuffer[0], sizeof(v));
        v[0] = *((float*)&vbuffer[0]);
        v[1] = *((float*)&vbuffer[4]);
        v[2] = *((float*)&vbuffer[8]);
        v[3] = *((float*)&vbuffer[12]);
        for(i = 0; i < 4; i++){
            Serial.println(v[i]);
            if(v[i] == 0.0){
                
                hover[i] = true ;  
            }else{
                hover[i] = false ;
            }
        }
        move = !(hover[0] && hover[1] && hover[2] && hover[3]);
        
        if(move){
            Serial.println("Movendo-se");
        }else{
            Serial.println("Parado");
        }
        this->ardrone.pcmdCommand(move, v);
        
        // Serial.println("Controle");
    }else if(type == "ROTZ"){
        

    }
}


void ArdroneESPRF::secureHealth(void){
    if(this->data.fligth_data.altitude > 3000){
        this->ardrone.land();
        delay(5000);
    }
}

void ArdroneESPRF::control_wz(float setpoint){
    float v[4], temp, input, sp;
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
    
    temp = this->data.fligth_data.psi/1000.0 ;
    input = radians(temp);
    sp = radians(setpoint);
    Serial.println(temp);
    v[3] = this->pid_wz.compute(input, sp);
    this->ardrone.pcmdCommand(true, v);
}