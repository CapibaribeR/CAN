#include "mbed.h"
#include "CANMsg.h"

#define  CAN_ID_TX 0x100
#define BLINKING_RATE     100ms
/* Communication protocols */
CAN can(PA_11, PA_12, 125000);


//Serial serial(USBTX, USBRX, 115200);

CANMsg txMsg;
CANMsg rxMsg;

/* Interrupt handlers */
void canTxISR();
/* General functions */
char counter = 0;
int RPM;
int16_t RPM_inter;
int16_t RPM_inter2;
volatile bool can_txisr = false;

int main()
{
    /* Main variables */
    can.attach(&canTxISR, CAN::TxIrq);
    printf("Enviar para o Inversor:\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);
    while (true) {
        can_txisr = true;
        if(can_txisr){
            can_txisr = false;
           //counter++;
           // led = !led;
            txMsg.clear();
            txMsg.len = 2;
            RPM = 42218;
            txMsg.data[0] = RPM >> 8;
            RPM_inter = RPM << 8;
            //RPM_inter = RPM_inter >> 8;
            //RPM = RPM_inter & 0x000000FF;
            txMsg.data[1] = RPM & 0xFF;
            can.write(txMsg);
            //printf("counter = %d\r\n",counter);
            printf("\r\n\t RPM HIGH = %d", txMsg.data[0]);
            printf("\r\n\t RPM LOW = %d", txMsg.data[1]);
            printf("\r\nEnviando Mensagem\r");
            printf("\r\n-----------------------------------\r\n");
        }
        printf("teste main loop\r\n");
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

void canTxISR() {
    can_txisr = true;
};
