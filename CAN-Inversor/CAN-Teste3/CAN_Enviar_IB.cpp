#include "mbed.h"
#include "CANMsg.h"

#define MAXRPM 9000
#define  CAN_ID_TX 0x100
#define BLINKING_RATE     100ms
/* Communication protocols */
CAN can(PA_11, PA_12, 125000);

InterruptIn incrementButton(PA_3);
AnalogIn sensor(PA_1);
DigitalOut led(LED1);
//Serial serial(USBTX, USBRX, 115200);

CANMsg txMsg;
CANMsg rxMsg;
void verifyButton();
/* Interrupt handlers */
void canTxISR();
/* General functions */
char counter = 0;
uint16_t rpm;
uint16_t pwm;
uint16_t corrente;
uint16_t RPM_inter;
uint16_t RPM_inter2;
volatile bool can_txisr = false;
bool button_flag = false;

int main()
{
    rpm = 0;
    /* Main variables */
    can.attach(&canTxISR, CAN::TxIrq);
    
    printf("Enviar para o Inversor:\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);
    while (true) {
    incrementButton.mode(PullDown);
    incrementButton.rise(&verifyButton);
        can_txisr = true;
    //Botão incrementar rpm
        if(button_flag == true){
                led = !led;
                button_flag = false;
                if(rpm < MAXRPM){
                rpm+=500;
                }else{
                    rpm = 0;
                }

            }

        if(can_txisr){
            can_txisr = false;
           //counter++;
           // led = !led;
            txMsg.clear();
            txMsg.len = 8;
            pwm = sensor.read_u16();
            //rpm = 42218;
            corrente = 30127;
            txMsg.data[0] = rpm >> 8;
            RPM_inter = rpm << 8;
            //RPM_inter = RPM_inter >> 8;
            //RPM = RPM_inter & 0x000000FF;
            txMsg.data[1] = rpm & 0xFF;
            txMsg.data[2] = rpm >> 8;
            txMsg.data[3] = pwm & 0xFF;
            txMsg.data[4] = pwm >> 8;
            txMsg.data[5] = corrente & 0xFF;
            txMsg.data[6] = corrente >> 8;
            txMsg.data[7] = 0b00000000; 
            can.write(txMsg);
            //printf("counter = %d\r\n",counter);
            printf("\r\n\t RPM HIGH = %d", txMsg.data[0]);
            printf("\r\n\t RPM LOW = %d", txMsg.data[1]);
            printf("\r\n\t RPM = %d", rpm);
            printf("\r\n\t PAR DE POLOS = %d", txMsg.data[2]);
            printf("\r\n\t PWM LOW = %d", txMsg.data[3]);
            printf("\r\n\t PWM HIGH = %d", txMsg.data[4]);
            printf("\r\n\t PMW = %d", pwm);
            printf("\r\n\t CORRENTE LOW = %d", txMsg.data[5]);
            printf("\r\n\t CORRENTE HIGH = %d", txMsg.data[6]);
            printf("\r\n\t CORRENTE = %d", corrente);
            printf("\r\n\t RPM LOW = %d", txMsg.data[7]);
            printf("\r\nEnviando Mensagem\r");
            printf("\r\n-----------------------------------\r\n");
        }
        printf("----");
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

void canTxISR() {
    can_txisr = true;
};

void verifyButton(){
    button_flag = true;
}
