#include "mbed.h"
#include "CANMsg.h"

#define CAN_ID_RX 0x101
#define BLINKING_RATE     100ms

/* Communication protocols */
CAN can(PA_11, PA_12, 125000);
//Serial serial(USBTX, USBRX, 115200);

CANMsg txMsg;
CANMsg rxMsg;

void canISR();
char counter = 0;
int RPM;
int RPM_HIGH;
int RPM_LOW;
volatile bool can_isr = false; 

int main()
{
    can.attach(&canISR, CAN::RxIrq);
    printf("Informações do Inversor:\r\n");
    printf("Mensagem inicializada: RX (ID: %x)\r\n", txMsg.id);
    while (true) {
        can_isr = true;
        if(can_isr){
            can_isr = false;
            can.read(rxMsg);
            RPM_HIGH = rxMsg.data[0];
            RPM_LOW = rxMsg.data[1]; 
            printf("\r\n\t RPM HIGH = %d", rxMsg.data[0]); //bits 4 ao 7 fazem a leitura da tensão de entrada (HIGH)
            printf("\r\n\t RPM LOW = %d", rxMsg.data[1]);
            printf("\r\n-----------------------------------");
            RPM = RPM_HIGH << 8 | RPM_LOW;
            printf("\r\n\t RPM = %d", RPM);
            // RPM = RPM_HIGH | RPM_LOW
            printf("\r\n-----------------------------------");
        }
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

void canISR() {
    can_isr = true;
}
