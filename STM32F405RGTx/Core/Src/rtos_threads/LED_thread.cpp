#include "main.h"
#include "timer_utils.h"
#include "threads.hpp"
#include "LED_thread.hpp"

RTOSThread LEDThread::thread_;
float LEDThread::R;
float LEDThread::G;
float LEDThread::B;
uint8_t LEDThread::rgb_blink;
uint8_t LEDThread::rgb_on;
uint32_t LEDThread::cnt;

void LEDThread::initialize() {
    LEDThread::thread_ = RTOSThread(
        "debug_led_thread",
        1024*3,
        LED_THREAD_PRIORITY,
        LEDThread::runLEDThread
    );
    LEDThread::R = 0;
    LEDThread::G = 0;
    LEDThread::B = 0;
    LEDThread::rgb_blink = 0;
    LEDThread::rgb_on = 0;
    LEDThread::cnt = 0;
}

void LEDThread::setLED(float R_, float G_, float B_, uint8_t blink_) {
    LEDThread::R = R_;
    LEDThread::G = G_;
    LEDThread::B = B_;
    LEDThread::rgb_blink = blink_;
    if (!blink_) {
        LEDThread::rgb_on = 1;
    }
}

void LEDThread::runLEDThread(void* args) {
    while (1) {
        // blink the debug LED 4 times slower than the thread refresh rate
        DEBUG_GPIO_Port->ODR ^= DEBUG_Pin*(cnt % 4);

        set_led_intensity(RED, LEDThread::R*LEDThread::rgb_on);
        set_led_intensity(GREEN, LEDThread::G*LEDThread::rgb_on);
        set_led_intensity(BLUE, LEDThread::B*LEDThread::rgb_on);

        LEDThread::rgb_on ^= LEDThread::rgb_blink;
        LEDThread::cnt += 1;

        osDelay(LED_THREAD_PERIODICITY);
    }
}


