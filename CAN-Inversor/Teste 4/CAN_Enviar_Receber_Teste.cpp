#include "mbed.h"
#include "CANMsg.h"
#define MAXRPM 9000
#define MAXPWM 65535
#define  CAN_ID_TX 0x100
#define CAN_ID_RX 0x101
#define BLINKING_RATE     600ms
/* Communication protocols */
CAN can(PA_11, PA_12, 125000);

InterruptIn incrementButton(PA_3);
InterruptIn decrementButton(PF_1);

InterruptIn incrementPwmButton(PA_0);
InterruptIn decrementPwmButton(PA_1);

DigitalOut led(LED1);
//Serial serial(USBTX, USBRX, 115200);

CANMsg txMsg;
CANMsg rxMsg;
void verifyButton();
void verifyDecrementButton();

void verifyDecrementPwmButton();
void verifyPwmButton();
/* Interrupt handlers */
void canISR();
void canRxISR();
/* General functions */

// Variáveis TX
char counter = 0;
uint16_t rpm = 0;
uint16_t pwm = 0;
float pwm_percent = 0;
uint16_t corrente = 260;
uint16_t RPM_inter;
uint16_t RPM_inter2;
volatile bool can_tx_isr = false;
volatile bool can_rx_isr = false;

//Variáveis RX
uint16_t rx_rpm;
uint16_t rpm_high;
uint8_t rpm_low;
uint8_t tensao_8b;
uint16_t tensao_aux;
uint16_t tensao;
uint8_t contador;
uint8_t temp_controlador;
uint8_t temp_motor;
uint8_t rx_pwm;
uint8_t rx_corrente;

// Botões
bool button_decFlag = false;
bool button_flag = false;
bool buttonPwm_decFlag = false;
bool buttonPwm_flag = false;

float to_porcent(int valor, int maximo, int minimo);
int main()
{
    rpm = 0;
    /* Main variables */
    can.attach(&canISR, CAN::TxIrq);
    can.attach(&canRxISR, CAN::RxIrq);


    printf("Enviar para o Inversor:\r\n");
    printf("Mensagem inicializada: TX (ID: %x)\r\n", txMsg.id);
    while (true) {

//Botões RPM
    incrementButton.mode(PullDown);
    incrementButton.rise(&verifyButton);

    decrementButton.mode(PullDown);
    decrementButton.fall(&verifyDecrementButton);
// Botões PWM
    incrementPwmButton.mode(PullDown);
    incrementPwmButton.rise(&verifyButton);

    decrementPwmButton.mode(PullDown);
    decrementPwmButton.fall(&verifyDecrementButton);


     ThisThread::sleep_for(BLINKING_RATE);
        
    //Botão incrementar rpm
        if(button_flag == true){
                led = !led;
                button_flag = false;
                if(rpm < MAXRPM){
                rpm+=500;
                }else{
                    rpm = MAXRPM;
                }
            }
    //Botão decrementar rpm
        if(button_decFlag == true){
                    button_decFlag = false;

                    if(rpm>0){
                        rpm-=500;
                        printf("%i\n",rpm);
                }else{

                   printf("%i\n",rpm);
                    rpm = 0;
                }
            }
    //Botão incrementar PWM +2000
            if(buttonPwm_flag == true){
                buttonPwm_flag = false;
                if(pwm < MAXPWM){
                pwm+=2000;
                }else{
                    pwm = MAXPWM;
                }
            }
    //Botão decrementar PWM -2000
        if(buttonPwm_decFlag == true){
                    buttonPwm_decFlag = false;

                    if(pwm>0){
                        pwm-=2000;
                }else{
                    pwm = 0;
                }
            }

        pwm_percent = (pwm/65535)*100;
        can_tx_isr = true;
        can_rx_isr = true;
        if(can_tx_isr){
            can_tx_isr = false;

            can.read(rxMsg);
            contador = rxMsg.data[0] >> 4;
            tensao_8b = rxMsg.data[0] << 4;
            tensao_aux = tensao_8b << 4;
            tensao = tensao_aux | rxMsg.data[1];
            temp_controlador = rxMsg.data[2];
            temp_motor = rxMsg.data[3];
            rpm_high = rxMsg.data[4];
            rpm_low = rxMsg.data[5];
            rx_rpm = rpm_high << 8 | rpm_low; 
            rx_pwm = rxMsg.data[6];
            rx_corrente = rxMsg.data[7];

            txMsg.clear();
            txMsg.len = 8;
            txMsg.data[0] = rpm & 0xFF; //  Byte menos significativos do RPM
            txMsg.data[1] = rpm >> 8; // Byte mais significativos do RPM
            txMsg.data[2] = 0; // Numero de pares de polos do motor
            txMsg.data[3] = pwm & 0xFF; // Byte menos significativo do PWM
            txMsg.data[4] = pwm >> 8; // Byte mais significativo do PWM
            txMsg.data[5] = corrente & 0xFF; // Byte menos significativo corrente;
            txMsg.data[6] = corrente >> 8; // Byte mais significativo da corrente;
            txMsg.data[7] = 0b00000000; 
            can.write(txMsg);

           
            printf("\r\n\t TX RPM HIGH = %d", txMsg.data[0]);
            printf("\r\n\t TX RPM LOW = %d", txMsg.data[1]);
            printf("\r\n\t TX RPM MAX = %d", rpm);
            printf("\r\n\t PAR DE POLOS = %d", txMsg.data[2]);
            printf("\r\n\t PWM LOW = %d", txMsg.data[3]);
            printf("\r\n\t PWM HIGH = %d", txMsg.data[4]);
            printf("\r\n\t PMW = %d %.2f", pwm, pwm_percent);
            printf("\r\n\t CORRENTE LOW = %d", txMsg.data[5]);
            printf("\r\n\t CORRENTE HIGH = %d", txMsg.data[6]);
            printf("\r\n\t CORRENTE MÁXIMA = %d", corrente);
            printf("\r\nEnviando Mensagem\r");

            printf("\r\n-----------------------------------\r\n");
            printf("\r\n-----------------------------------\r\n");

            printf("\r\n\nMensagem Recebida Inversor: \r\n");
            printf("\r\n\t CONTADOR = %d", contador);
            printf("\r\n\t TENSÃO = %d", tensao);
            printf("\r\n\t TEMP CONTROLADOR = %d", temp_controlador); //bits 4 ao 7 fazem a leitura da tensão de entrada (HIGH)
            printf("\r\n\t TEMP MOTOR = %d", temp_motor);
            printf("\r\n\t RPM = %d", rx_rpm); //RPM representado em 2 bytes
            printf("\r\n\t PWM = %d", rx_pwm); //0-255 mapeado para 0-100%
            printf("\r\n\t CORRENTE = %d", corrente);
            // RPM = RPM_HIGH | RPM_LOW
            printf("\r\n-----------------------------------");
        }

        if(can_rx_isr){
            can_rx_isr = false;

            can.read(rxMsg);
            contador = rxMsg.data[0] >> 4;
            tensao_8b = rxMsg.data[0] << 4;
            tensao_aux = tensao_8b << 4;
            tensao = tensao_aux | rxMsg.data[1];
            temp_controlador = rxMsg.data[2];
            temp_motor = rxMsg.data[3];
            rpm_high = rxMsg.data[4];
            rpm_low = rxMsg.data[5];
            rx_rpm = rpm_high << 8 | rpm_low; 
            rx_pwm = rxMsg.data[6];
            rx_corrente = rxMsg.data[7];

            printf("\r\n\nMensagem Recebida Inversor: \r\n");
            printf("\r\n\t CONTADOR = %d", contador);
            printf("\r\n\t TENSÃO = %d", tensao);
            printf("\r\n\t TEMP CONTROLADOR = %d", temp_controlador); //bits 4 ao 7 fazem a leitura da tensão de entrada (HIGH)
            printf("\r\n\t TEMP MOTOR = %d", temp_motor);
            printf("\r\n\t RPM = %d", rx_rpm); //RPM representado em 2 bytes
            printf("\r\n\t PWM = %d", rx_pwm); //0-255 mapeado para 0-100%
            printf("\r\n\t CORRENTE = %d", corrente);
            // RPM = RPM_HIGH | RPM_LOW
            printf("\r\n-----------------------------------");
        }
        printf("----");
        ThisThread::sleep_for(BLINKING_RATE);
    }

    //END OF MAIN
}

void canISR() {
    can_tx_isr = true;
};

void canRxISR() {
    can_rx_isr = true;
};

void verifyButton(){
    button_flag = true;
}

void verifyDecrementButton(){
    button_decFlag = true;
}

void verifyPwmButton(){
    buttonPwm_flag = true;
}

void verifyDecrementPwmButton(){
    buttonPwm_decFlag = true;
}
