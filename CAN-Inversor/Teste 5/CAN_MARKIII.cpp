#include "mbed.h"
#include "CAN.h"

#define INVERSOR_TX_ID 0x100
#define INVERSOR_RX_ID 0x101
#define MAXRPM 9000
#define MAXPWM 65000
#define MAX_CAN_DATA_SIZE 8

//CAN
CAN can1(PA_11, PA_12, 125000);
bool is_can_active();
void reset_can();

//Função para enviar as mensagem CAN para o inversor
void send_to_inverter();
// Função para receber dados do inversor
void receive_from_inverter();
// Fim de CAN

// Variáveis do TX
char counter = 0;
uint16_t rpm = 0;
uint16_t pwm = 0;
float pwm_percent = 0.0;
float pwm_aux = 0.0;
uint16_t corrente = 260;
// Variáveis de STATUS - RX
uint16_t rx_rpm;
uint16_t rpm_aux;
uint8_t rpm_high;
uint8_t rpm_low;
uint8_t tensao_high;
uint16_t tensao_aux;
uint16_t tensao;
uint8_t contador;
uint8_t temp_controlador;
uint8_t temp_motor;
uint8_t rx_pwm;
uint8_t rx_corrente;
float tensao_float;


// Botões
bool button_decFlag = false;
bool button_flag = false;
bool buttonPwm_decFlag = false;
bool buttonPwm_flag = false;

void verifyButton();
void verifyDecrementButton();
void verifyDecrementPwmButton();
void verifyPwmButton();

InterruptIn incrementButton(PA_3);
InterruptIn decrementButton(PA_4);
InterruptIn incrementPwmButton(PA_0);
InterruptIn decrementPwmButton(PA_1);

void Buttons();
//Fim de Botões





// main() runs in its own thread in the OS




int main()
{
    can1.mode(CAN::Normal);
    can1.filter(0, 0, CANStandard);

    
    while (true) 

    {


        
        Buttons(); // Chamamento da função dos botões

        send_to_inverter(); // Chamamento da função pra enviar ao inversor
        receive_from_inverter(); // Chamento da Função para receber do Inversor

        wait_us(1e5); // aguarda 100ms
        
    }
}
//end of main




void receive_from_inverter() {
    //Definir mensagem CAN a ser recebida
    CANMessage inverter_rx_msg;
    inverter_rx_msg.len = 8;
    
if(is_can_active()) {
    if(can1.read(inverter_rx_msg)){
    // Aguardar a recepção da mensagem do inversor

    if(inverter_rx_msg.id == INVERSOR_RX_ID){

    contador = inverter_rx_msg.data[0] & 0xF;
    tensao_high = inverter_rx_msg.data[0] >> 4;
    tensao_aux = tensao_high << 8;

    tensao = tensao_aux | inverter_rx_msg.data[1];

    temp_controlador = inverter_rx_msg.data[2];
    temp_motor = inverter_rx_msg.data[3];
    rpm_low = inverter_rx_msg.data[4];
    rpm_high = inverter_rx_msg.data[5];
    rx_pwm = inverter_rx_msg.data[6];
    rx_corrente = inverter_rx_msg.data[7];
    
    rpm_aux = rpm_high << 8;
    rx_rpm = rpm_low | rpm_aux; 

    tensao_float = tensao/10;


    printf("\r\n\nMensagem Recebida Inversor: RX (ID: 0x%x)\r\n",inverter_rx_msg.id);
    printf("\r\n\t CONTADOR = %d", contador);
    printf("\r\n\t TENSÃO = %.1f V", tensao_float);
    printf("\r\n\t TEMP CONTROLADOR = %d°C", temp_controlador); //bits 4 ao 7 fazem a leitura da tensão de entrada (HIGH)
    printf("\r\n\t TEMP MOTOR = %d°C ", temp_motor);
    printf("\r\n\t RPM = %d rpm", rx_rpm); //RPM representado em 2 bytes
    printf("\r\n\t PWM = %d pwm", rx_pwm); //0-255 mapeado para 0-100%
    printf("\r\n\t CORRENTE = %d A", rx_corrente);
    printf("\r\n-----------------------------------");
    } else{
        printf("\r\n\nMensagem Recebida Inversor: RX (ID: 0x%x)\r\n",inverter_rx_msg.id);
    }
    }
 } else {
            printf("Conexão CAN inativa. Tentando reinicializar...\n");
            reset_can(); 
        }
   }
    


void send_to_inverter(){
    
    CANMessage inverter_tx_msg;
    inverter_tx_msg.id = INVERSOR_TX_ID;
    inverter_tx_msg.len = 8; // Define o tamanho da msg, max = 8 bits
 
    printf("\r\n\nEnviar para o Inversor:\r\n");
    printf("Mensagem inicializada: TX (ID: 0x%x)\r\n", inverter_tx_msg.id);

    inverter_tx_msg.data[0] = rpm & 0xFF; //  Byte menos significativos do RPM
    inverter_tx_msg.data[1] = rpm >> 8; // Byte mais significativos do RPM
    inverter_tx_msg.data[2] = 0; // Numero de pares de polos do motor
    inverter_tx_msg.data[3] = pwm & 0xFF; // Byte menos significativo do PWM
    inverter_tx_msg.data[4] = pwm >> 8; // Byte mais significativo do PWM
    inverter_tx_msg.data[5] = corrente & 0xFF; // Byte menos significativo corrente;
    inverter_tx_msg.data[6] = corrente >> 8; // Byte mais significativo da corrente;
    inverter_tx_msg.data[7] = 0b00000000; 

    //Enviar a mensagem CAN para o inversor

    if(can1.write(inverter_tx_msg)){

    pwm_aux = pwm;
    pwm_percent = (pwm_aux/MAXPWM)*100;

    printf("\r\n\t TX RPM HIGH = %d", inverter_tx_msg.data[0]);
    printf("\r\n\t TX RPM LOW = %d", inverter_tx_msg.data[1]);
    printf("\r\n\t TX RPM MAX = %d", rpm);
    printf("\r\n\t PAR DE POLOS = %d", inverter_tx_msg.data[2]);
    printf("\r\n\t PWM LOW = %d", inverter_tx_msg.data[3]);
    printf("\r\n\t PWM HIGH = %d", inverter_tx_msg.data[4]);
    printf("\r\n\t PMW = %d %.2f%%", pwm,pwm_percent);
    printf("\r\n\t CORRENTE LOW = %d", inverter_tx_msg.data[5]);
    printf("\r\n\t CORRENTE HIGH = %d", inverter_tx_msg.data[6]);
    printf("\r\n\t CORRENTE MÁXIMA = %d", corrente);
    printf("\r\nEnviando Mensagem\r");

    printf("\r\n-----------------------------------\r\n");
    printf("\r\n-----------------------------------\r\n");

    } else{
        printf("Não foi possível enviar a mensagem CAN");
    }
    }


void Buttons(){

    incrementButton.mode(PullDown);
    incrementButton.rise(&verifyButton);

    decrementButton.mode(PullDown);
    decrementButton.fall(&verifyDecrementButton);

    incrementPwmButton.mode(PullDown);
    incrementPwmButton.rise(&verifyPwmButton);

    decrementPwmButton.mode(PullDown);
    decrementPwmButton.fall(&verifyDecrementPwmButton);


    //Botão incrementar rpm
    if(button_flag == true){
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
                }else{
                    rpm = 0;
                }
            }
    //Botão incrementar PWM +2000
            if(buttonPwm_flag == true){
                buttonPwm_flag = false;
                if(pwm < MAXPWM){
                pwm+=5000;
                }else{
                    pwm = MAXPWM;
                }
            }
    //Botão decrementar PWM -2000
        if(buttonPwm_decFlag == true){
                    buttonPwm_decFlag = false;

                    if(pwm>0){
                        pwm-=5000;
                }else{
                    pwm = 0;
                }
            }
}

bool is_can_active(){
    CANMessage msg;
    return can1.read(msg); // tente ler uma mensagem
}

void reset_can() {
    can1.reset(); // redefinir a interface CAN
    can1.mode(CAN::Normal);
    can1.filter(0, 0, CANStandard); // reconfigurar o filtro para receber todas as mensagens
}

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

