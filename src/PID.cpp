#include <ESP8266WiFi.h>
class PIDControl{
    public:
        // Ganhos PID
        float kp,ki,kd;

        // Componentes PID
        float P,I,D;

        // Last Time and Last Input (OBS: Necessario para PI e PID)
        float ltime, linput;

        // Max and Min
        float min,max;

        PIDControl(void){

            this->kp = 1;
            this->ki = 0;
            this->kd = 0;

            this->P = 0;
            this->I = 0;
            this->D = 0;
            
            this->ltime = 0;
            this->linput = 0;
        }
        void setTunings(float kp, float ki, float kd){
            this->kp = kp;
            this->ki = ki;
            this->kd = kd;
        
        }
        void setLimits(float min, float max){
            this->min = min;
            this->max = max;
        }

        float compute(float input, float setpoint){
            float error, dt, di, output;

            error = setpoint - input;
            
            // Variacao do tempo(s) Segundos
            dt = (millis() - this->ltime)/1000.0; 
            this->ltime = millis();

            // Variacao do Input
            di = input - this->linput;
            this->linput = input;

            // Proporcional Control
            this->P = error*this->kp;
            
            // Integrative Control
            if(this->ki == 0){
                this->I = 0;
            }else{
                this->I = this->I + (error*this->ki)*dt;
                if(this->I > this->max){
                    this->I  = this->max;
                }else if(this->I < this->min){
                    this->I  = this->min;
                }
            }
            // Derivative Control
            if(this->kd == 0){
                this->D = 0;
            }else{
                this->D = error*(di/dt)*this->kd;
            }

            // Composicao da Saida
            // print("P = %s, I = %s"%(this->P,this->I))

            output = this->P + this->I + this->D;

            if(output > this->max){
                output = this->max;
                return output;
            }else if(output < this->min){
                output = this->min;
                return output;
            }else{
                return output;
            }


        }



};

  