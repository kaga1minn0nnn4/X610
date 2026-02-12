#pragma once

#include "delay.h"
#include "RUR_STM_G4Lib/version.hpp"
#include "RUR_STM_G4Lib/Pin.hpp"
#include "RUR_STM_G4Lib/GPIO.hpp"
#include "RUR_STM_G4Lib/TIM.hpp"
#include "RUR_STM_G4Lib/UART.hpp"
#include "RUR_STM_G4Lib/SPI.hpp"
#include "RUR_STM_G4Lib/other.hpp"
#include "RUR_STM_G4Lib/OPAMP.hpp"
#include "RUR_STM_G4Lib/ADCv2.hpp"
#include "RUR_STM_G4Lib/CANFD.hpp"

#include "math.h"


namespace x610 {

constexpr uint32_t kTimerInterruptFreq = 1000;
constexpr uint32_t kPWMTimerFreq = 40000;
constexpr uint32_t kEscControlFreq = 20000;

extern G4::UART serial;
extern G4::TIM it_timer;
extern G4::TIM pwm_timer;
extern G4::TIM esc_control_timer;
extern std::array<G4::PWM, 3> pwms;
extern std::array<G4::OpAMP, 3> opamps;
extern std::array<G4::ADCv2, 2> adcs;
extern G4::CANFD canfd;
extern G4::GPIO n_fault;
extern G4::GPIO drvoff;
extern G4::LED user_led;
extern G4::PushSensor user_sw;
extern G4::PushSensor limit_sw;

extern std::vector<uint16_t> sensor_value_raw_adc1;
extern std::vector<uint16_t> sensor_value_raw_adc2;

extern void it_timer_task();
extern void ctl_timer_task();

bool config();

void start();

}
