#include "mbed.h"
#include "CANMsg.h"

#define VOLTAGE_ID 0xE1
#define BLINKING_RATE     1000ms
/* Communication protocols */
CAN can(PA_11, PA_12, 125000);


//Serial serial(USBTX, USBRX, 115200);

CANMsg txMsg;
CANMsg rxMsg;

/* Interrupt handlers */
void canTxISR();
/* General functions */
char counter = 0;

volatile bool can_txisr = false;

int main()
{
    /* Main variables */
    can.attach(&canTxISR, CAN::TxIrq);
    printf("Oi gente...\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);
    while (true) {
        can_txisr = true;
        if(can_txisr){
            can_txisr = false;
            counter++;
           // led = !led;
            txMsg.clear();
            txMsg.len = 1;
            txMsg.data[0] = counter;
            can.write(txMsg);
            printf("counter = %d\r\n",counter);
            printf("\r\n\t V0 = %d", txMsg.data[0]);
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