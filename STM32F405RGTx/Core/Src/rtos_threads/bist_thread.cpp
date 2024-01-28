#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "threads.hpp"
#include "bms_entry.hpp"
#include "timer_utils.h"
#include "state_machine.hpp"
#include "bist_thread.hpp"
#include "bsp.h"

static const char* _banner = " _       __      __            __                      \r\n" \
                             "| |     / /___ _/ /____  _____/ /___  ____  ____       \r\n" \
                             "| | /| / / __ `/ __/ _ \\/ ___/ / __ \\/ __ \\/ __ \\  \r\n" \
                             "| |/ |/ / /_/ / /_/  __/ /  / / /_/ / /_/ / /_/ /      \r\n" \
                             "|__/|__/\\__,_/\\__/\\___/_/  /_/\\____/\\____/ .___/  \r\n" \
                             "                                        /_/            \r\n";

RTOSThread BistThread::thread_;
void (*BistThread::callback)(void);

// Button ISR
// I'm not putting too much effort into making this ISR short, since we are inherantly
// not in a time/safety sensitive environment if we are using the BIST...
static uint32_t prev_time = 0;
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == BIST_Pin) {
        uint32_t curr_time = HAL_GetTick();
        if ( (HAL_GetTick() - prev_time) > BUTTON_DEBOUNCE_TIME ) {
            BistThread::toggleBist();
            prev_time = curr_time;
        }
    }
}


void BistThread::initialize() {
    BistThread::thread_ = RTOSThread(
        "bist_thread",
        1024,
        BIST_THREAD_PRIORITY,
        BistThread::runBist
    );
    BistThread::callback = &BistThread::disabled_callback;
}

void BistThread::toggleBist() {
    if (BistThread::callback == &BistThread::enabled_callback) {
        BistThread::callback = &BistThread::disabled_callback;
    }
    else if (BistThread::callback == &BistThread::disabled_callback) {
        printf("%s", _banner);
        printf("Master BMS BIST Console\r\n");
        printf("Type 'help' for a list of available commands...\r\n\r\n");
        BistThread::callback = &BistThread::enabled_callback;
    }
}

static uint8_t buff[20];
static uint32_t len = 20;
static bool admin = false;
void BistThread::enabled_callback() {
    BistThread::_sinput("> ", buff, &len);

    if (BistThread::_strcmp(buff, "p_measurements"))                   { BistThread::_p_measurements(); }
    else if (BistThread::_strcmp(buff, "pm"))                          { BistThread::_p_measurements(); }
    else if (BistThread::_strcmp(buff, "rgb"))                         { BistThread::_rgb(); }
    else if (BistThread::_strcmp(buff, "help"))                        { BistThread::_help(); }
    else if (BistThread::_strcmp(buff, "toggle_fc"))                   { BistThread::_toggle_fc(); }
    else if (BistThread::_strcmp(buff, "clear"))                       { BistThread::_clear(); }
    else if (BistThread::_strcmp(buff, "admin"))                       { admin = !admin; }
    else if (BistThread::_strcmp(buff, "bms_state")&&admin==true)      { BistThread::_bms_state(); }
    else if (BistThread::_strcmp(buff, "toggle_cont")&&admin==true)    { BistThread::_toggle_cont(); }
    else if (BistThread::_strcmp(buff, ""))                            { /* do nothing... */ }

    else { printf("invalid command...\r\n"); }

    len = 20;
}
void BistThread::disabled_callback() { osThreadYield(); }

void BistThread::runBist(void* args) {
    // this implementation of disabling the thread is kinda poo poo but it should also be fine...
    while (1) {
        BistThread::callback();
    }
}

void BistThread::_print(uint8_t* str) {
    uint32_t len = 0;
    while (str[len] != '\0') { len += 1; }

    HAL_UART_Transmit(&huart1, str, len, 1);
}


void BistThread::_sinput(const char* prompt, uint8_t* buff, uint32_t* len) {
    BistThread::_print((uint8_t*)prompt);
    uint32_t curr_len = 0;

    while (curr_len < (*len - 1)) {
        uint8_t tmp;

        // try to receive 1 character with a timeout value of 1ms
        HAL_StatusTypeDef status = HAL_UART_Receive(&huart1, &tmp, 1, 0);

        if (status == HAL_TIMEOUT) {
            // do nothing...
        }
        else if (status == HAL_OK) {
            HAL_UART_Transmit(&huart1, &tmp, 1, 0);
            if (tmp == '\n') {
                break;
            }
            else if (tmp == '\r') {
                BistThread::_print((uint8_t*)"\n");
                break;
            }
            else {
                buff[curr_len] = tmp;
                curr_len += 1;
            }
        }
        else if (status == HAL_ERROR) {
            Error_Handler();
        }

        // humans cannot type faster than 50ms per character...
        // if you can, then too bad!!!
        osDelay(BIST_THREAD_PERIODICITY);
    }
    buff[curr_len++] = '\0';
    *len = curr_len;
}

uint8_t BistThread::_strcmp(uint8_t* a, const char* b) {
    return !strcmp((const char*)a, b);
}

void BistThread::_help() {
    BistThread::_print((uint8_t*)"p_measurements [pm]   --> print BMS measurements to the screen\r\n");
    BistThread::_print((uint8_t*)"rgb                   --> change the color of the RGB LED\r\n");
    BistThread::_print((uint8_t*)"toggle_fc             --> toggle fault checking in state machine\r\n");
    BistThread::_print((uint8_t*)"clear                 --> clears the command interface\r\n");
    BistThread::_print((uint8_t*)"bms_state             --> change the BMS state in state machine\r\n");
    BistThread::_print((uint8_t*)"toggle_cont           --> toggles the contactor to OFF\r\n");
}

void BistThread::_p_measurements() {
    // not done yet...
    printf("bms.buck_temp = %d deg C\r\n", (int)global_bms_data.buck_temp);
    printf("bms.mc_cap_voltage = %dmV\r\n", (int)(global_bms_data.mc_cap_voltage*1000));
    printf("bms.contactor_voltage = %dmV\r\n", (int)(global_bms_data.contactor_voltage*1000));
    printf("bms.bms_current = %dmA\r\n", (int)(global_bms_data.bms_current*1000));
    printf("bms.battery.voltage = %dmV\r\n", (int)(global_bms_data.battery.voltage*1000));
    printf("bms.battery.current = %dmA\r\n", (int)(global_bms_data.battery.current*1000));
    printf("bms.battery.soc = %d%%\r\n", (int)(global_bms_data.battery.soc));
}
void BistThread::_rgb() {
    set_led_intensity(RED, 0);
    set_led_intensity(GREEN, 0);
    set_led_intensity(BLUE, 0);

    uint8_t buff[5];
    uint32_t len = 5;

    BistThread::_sinput("R: ", buff, &len);
    float red = atof((const char*)buff);
    set_led_intensity(RED, red);
    len = 5;

    BistThread::_sinput("G: ", buff, &len);
    float green = atof((const char*)buff);
    set_led_intensity(GREEN, green);
    len = 5;

    BistThread::_sinput("B: ", buff, &len);
    float blue = atof((const char*)buff);
    set_led_intensity(BLUE, blue);

}

void BistThread::_toggle_fc() {
    uint8_t buff[10];
    uint32_t len = 10;

    while (true) {
        BistThread::_sinput("on or off?: ", buff, &len);
        if (BistThread::_strcmp(buff, "on")) { 
            StateMachineThread::setFaultChecking(1);
            break;
        } else if (BistThread::_strcmp(buff, "off")) {
            StateMachineThread::setFaultChecking(0);
            break;
        } 
    }
}

void BistThread::_clear() {
    printf("\033[2J\r\n");
}

void BistThread::_bms_state(){
	uint8_t buff[20];
	uint32_t len = 20;

	while (true) {
		BistThread::_sinput("enter BMS state...", buff, &len);
		if (BistThread::_strcmp(buff, "initialize"))                { StateMachineThread::setState(Initialize); }
		else if (BistThread::_strcmp(buff, "initializeFault"))      { StateMachineThread::setState(InitializeFault); }
		else if (BistThread::_strcmp(buff, "idle"))                 { StateMachineThread::setState(Idle); }
		else if (BistThread::_strcmp(buff, "precharging"))          { StateMachineThread::setState(Precharging); }
		else if (BistThread::_strcmp(buff, "run"))                  { StateMachineThread::setState(Run); }
		else if (BistThread::_strcmp(buff, "stop"))                 { StateMachineThread::setState(Stop); }
		else if (BistThread::_strcmp(buff, "sleep"))                { StateMachineThread::setState(Sleep); }
		else if (BistThread::_strcmp(buff, "normalDangerFault"))    { StateMachineThread::setState(NormalDangerFault); }
		else if (BistThread::_strcmp(buff, "severeDangerFault"))    { StateMachineThread::setState(SevereDangerFault); }
		else if (BistThread::_strcmp(buff, "noFault"))              { StateMachineThread::setState(NoFault); }
		else if (BistThread::_strcmp(buff, "charging"))             { StateMachineThread::setState(Charging); }
		else if (BistThread::_strcmp(buff, "charged"))              { StateMachineThread::setState(Charged); }
		else if (BistThread::_strcmp(buff, "balancing"))            { StateMachineThread::setState(Balancing); }
		else if (BistThread::_strcmp(buff, ""))                     { /* do nothing... */ }
		else if (BistThread::_strcmp(buff, "help"))                 { BistThread::_help_state(); }
		else if (BistThread::_strcmp(buff, "exit"))                 { break; }

		else { printf("invalid command...\r\n"); }
	}
}

void BistThread::_help_state(){
	BistThread::_print((uint8_t*)"BMS States: \r\n - initialize \r\n - initializeFault \r\n - idle \r\n");
	BistThread::_print((uint8_t*)" - precharging \r\n - run \r\n - stop \r\n - sleep \r\n - normalDangerFault \r\n");
	BistThread::_print((uint8_t*)" - severeDanger \r\n - Fault \r\n - noFault \r\n - charging \r\n - charged \r\n");
	BistThread::_print((uint8_t*)" - balancing \r\n - *enter 'exit' to leave prompt\r\n");
}

void BistThread::_toggle_cont(){
	TURN_OFF_CONT1_PIN();
}
