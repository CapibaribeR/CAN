#include "mbed.h"
#include "CANMsg.h"

#define VOLTAGE_ID 0xE1
#define BLINKING_RATE     100ms

/* Communication protocols */
CAN can(PD_0, PD_1, 125000);

AnalogOut saida(PA_4);

//Serial serial(USBTX, USBRX, 115200);
CANMsg txMsg;
CANMsg rxMsg;

/* Interrupt handlers */
void canISR();

/* General functions */
char counter = 0;
volatile bool can_isr = false; 

int main()
{
    /* Main variables */
    can.attach(&canISR, CAN::RxIrq);

    printf("Oi gente...\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);

    while (true) {
        can_isr = true;
        if(can_isr){
            can_isr = false;
            can.read(rxMsg);
            printf("\r\n\t V0 = %d\n\t", rxMsg.data[0]);
            saida.write_u16(rxMsg.data[0] * 65535.0 / 255.0);
            printf("\r\n-----------------------------------\r\n");
        }
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

void canISR() {
    can_isr = true;
}