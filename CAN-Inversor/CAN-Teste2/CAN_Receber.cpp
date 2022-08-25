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
uint16_t rpm;
uint16_t rpm_high;
uint8_t rpm_low;
uint8_t tensao_8b;
uint16_t tensao_aux;
uint16_t tensao;
uint8_t contador;
uint8_t temp_controlador;
uint8_t temp_inversor;
uint8_t pwm;
uint8_t corrente;
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
            contador = rxMsg.data[0] >> 4;
            tensao_8b = rxMsg.data[0] << 4;
            tensao_aux = tensao_8b << 4;
            tensao = tensao_aux | rxMsg.data[1];
            temp_controlador = rxMsg.data[2];
            temp_inversor = rxMsg.data[3];
            rpm_high = rxMsg.data[4];
            rpm_low = rxMsg.data[5];
            rpm = rpm_high << 8 | rpm_low; 
            pwm = rxMsg.data[6];
            corrente = rxMsg.data[7];
            // TENSÃO = TENSÃO_HIGH << 4 | TENSÃO_LOW
            // CONTADOR = TENSÃO_HIGH >> 4
            printf("\r\n\t CONTADOR = %d", contador);
            printf("\r\n\t TENSÃO = %d", tensao);
            printf("\r\n\t TEMP CONTROLADOR = %d", temp_controlador); //bits 4 ao 7 fazem a leitura da tensão de entrada (HIGH)
            printf("\r\n\t TEMP INVERSOR = %d", temp_inversor);
            printf("\r\n\t RPM = %d", rpm); //RPM representado em 2 bytes
            printf("\r\n\t PWM = %d", pwm); //0-255 mapeado para 0-100%
            printf("\r\n\t CORRENTE = %d", corrente);
            // RPM = RPM_HIGH | RPM_LOW
            printf("\r\n-----------------------------------");
        }
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
void canISR() {
    can_isr = true;
}
