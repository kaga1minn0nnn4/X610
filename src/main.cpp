#include "main.hpp"
#include "board.hpp"
#include "controller.hpp"
#include "debug.hpp"

uint32_t adc_count = 0;

void x610_hardware::it_timer_task() {
    static uint32_t count = 0;
    if (count++ > 1000) {
        count = 0;
        user_led.toggle();

        // controller.printSensorValue();
        controller.resetcount();
    }

    static uint32_t sensor_count = 0;
    if (sensor_count++ > 100) {
        sensor_count = 0;
    }
}

void x610_hardware::adc_task() {
    controller.updateSensorValue();
}

bool sw_status_last = false;
bool driver_status = false;

float duty_out = 0.2f;
float freq_out = 10.0f;
void outputDuty(float alpha, float beta){
	float norm = sqrt(alpha * alpha + beta * beta);
	if(norm > 1.0f) {
		alpha /= norm;
		beta /= norm;
		norm = 1.0f;
	}
	float u = alpha * board::x610::receiver::kVectorU[0] + beta * board::x610::receiver::kVectorU[1];
	float v = alpha * board::x610::receiver::kVectorV[0] + beta * board::x610::receiver::kVectorV[1];
	float w = alpha * board::x610::receiver::kVectorW[0] + beta * board::x610::receiver::kVectorW[1];

    x610_hardware::pwms[0].setDuty(u * 90);
    x610_hardware::pwms[1].setDuty(v * 90);
    x610_hardware::pwms[2].setDuty(w * 90);
}
void main_process(void) {
    x610_hardware::config();
    controller.config();

    while (true) {
        auto key = x610_hardware::serial.read().value_or(0);
        if (key == '\n' || key == '\r') {
            debug.setup();
            break;
        }
    }

    while (true) {
        debug.run();

		volatile float angle_out = delay_getCount() / 170000000.0 * 2.0f * 3.14159265 * freq_out;
        volatile float test = angle_out;
		outputDuty(duty_out * cos(angle_out), duty_out * sin(angle_out));
    }
}
