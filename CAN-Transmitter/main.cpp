#include "mbed.h"
#include "CANMsg.h"

#define VOLTAGE_ID 0xE1
#define BLINKING_RATE     100ms

/* Communication protocols */
CAN can(PA_11, PA_12, 125000);

AnalogIn sensor(PA_1);

//Serial serial(USBTX, USBRX, 115200);
CANMsg txMsg;
CANMsg rxMsg;

/* Interrupt handlers */
void canTxISR();

/* General functions */
volatile bool can_txisr = false;

int main()
{
    /* Main variables */
    int value;
    can.attach(&canTxISR, CAN::TxIrq);
    sensor.set_reference_voltage(3.3);

    printf("Oi gente...\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);

    while (true) {
        can_txisr = true;
        if(can_txisr){
            can_txisr = false;
            txMsg.clear();
            txMsg.len = 1;
            value = rint(sensor.read_voltage() * 255 / 3.3);
            txMsg.data[0] = value;
            can.write(txMsg);
            printf("\r\n\t V0 = %d", txMsg.data[0]);
            printf("\r\n-----------------------------------\r\n");
        }
        ThisThread::sleep_for(BLINKING_RATE);
    }
}

void canTxISR() {
    can_txisr = true;
};
