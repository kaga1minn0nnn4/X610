#include "board.hpp"

namespace x610_hardware {

G4::UART serial{G4::uart3, G4::PC10, G4::PC11};

G4::TIM it_timer{G4::TimerClockParameter::generate<G4::tim2, kTimerInterruptFreq>()};
G4::TIM pwm_timer{G4::TimerClockParameter::generate<G4::tim1, kPWMTimerFreq>()};

std::array<G4::PWM, 3> pwms;

std::array<G4::OpAMP, 3> opamps {
    G4::OpAMP{G4::opamp1, G4::PA1, G4::PA3}, // u
    G4::OpAMP{G4::opamp2, G4::PA7, G4::PA5}, // v
    G4::OpAMP{G4::opamp3, G4::PB0, G4::PB2}, // w
};

std::array<G4::ADCv2, 2> adcs {
    G4::ADCv2{G4::adc1, {G4::PB12, G4::PB11}},
    G4::ADCv2{G4::adc2, {}}
};

G4::CANFD canfd{G4::canfd1, G4::PA12, G4::PA11};

G4::GPIO n_fault{G4::PC15};
G4::GPIO drvoff{G4::PC13};
G4::LED user_led{G4::PB7};
G4::PushSensor user_sw{G4::PB9};
G4::GPIO n_sleep{G4::PA0};

G4::Cordic cordic{G4::cordic};

std::vector<uint16_t> sensor_value_raw(2);
std::vector<uint16_t> dummy;

void enable_driver() {
    // なんかドライバONにした後にPWM出力始めないとnFAULT吐いて落ちる

    for (auto& pwm : x610_hardware::pwms) {
        pwm.setEnable(false);
    }
    delay_ms(10);

    x610_hardware::drvoff.write(false);

    delay_ms(100);

    for (auto& pwm : x610_hardware::pwms) {
        pwm.setEnable(true);
    }
}

void disable_driver() {
    x610_hardware::drvoff.write(true);
}

bool config() {
    bool result = true;
    result &= serial.config(115200);
    if (result) {
        serial << "\n";
        serial << "---------------------------\n";
        serial << "X610 v1.0.0\n";
        serial << "Start configuration !!!\n\n";
    }

    result &= drvoff.config(peripheral::gpio::Mode::out);
    drvoff.write(true); // DRV8328 OFF

    result &= n_fault.config(peripheral::gpio::Mode::in);
    result &= user_led.config();
    result &= user_sw.config();
    result &= n_sleep.config(peripheral::gpio::Mode::out);
    n_sleep.write(true);
    delay_ms(1);
    // n_sleep.write(false);
    delay_ms(1);
    n_sleep.write(true);
    serial << "GPIO init...\t" << result << "\n";

    result &= G4::itTimerConfig(it_timer, 10, it_timer_task);
    serial << "TIM2 init...\t" << result << "\n";

    result &= pwm_timer.config(peripheral::tim::CounterMode::triangle, 1);
    serial << "TIM1 init...\t" << result << "\n";

    result &= pwm_timer.configOC(G4::TIM::Ch::_1, G4::TIM::PWMMode::complementary, 0.8e-6);
    result &= pwm_timer.configOC(G4::TIM::Ch::_2, G4::TIM::PWMMode::complementary, 0.8e-6);
    result &= pwm_timer.configOC(G4::TIM::Ch::_3, G4::TIM::PWMMode::complementary, 0.8e-6);
    serial << "TIM1 Channel1~3 init...\t" << result << "\n";

    result &= pwms[0].config(pwm_timer.createChannel(G4::TIM::Ch::_1), G4::PA8, G4::PB13);
    result &= pwms[1].config(pwm_timer.createChannel(G4::TIM::Ch::_2), G4::PA9, G4::PB14);
    result &= pwms[2].config(pwm_timer.createChannel(G4::TIM::Ch::_3), G4::PA10, G4::PB15);
    serial << "PWM init...\t" << result << "\n";

    for (auto& opamp : opamps) {
        result &= opamp.config(peripheral::opamp::Mode::pga_io0_bias, peripheral::opamp::Gain::_16_or_minus_15);
    }
    serial << "OPAMP1~3 init...\t" << result << "\n";

    result &= adcs[0].config({LL_ADC_CHANNEL_11, LL_ADC_CHANNEL_14}, {LL_ADC_CHANNEL_VOPAMP1}, sensor_value_raw, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    result &= adcs[1].config({}, {LL_ADC_CHANNEL_VOPAMP2, LL_ADC_CHANNEL_VOPAMP3_ADC2}, dummy, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    serial << "ADCv2 init...\t" << result << "\n";

    result &= canfd.config(peripheral::canfd::Format::fd, peripheral::canfd::Mode::normal, 5e6, 5e6);
    serial << "CANFD1 init...\t" << result << "\n";

    result &= cordic.config();
    serial << "CORDIC init...\t" << result << "\n";

    serial << "\n";
    serial << "---------------------------\n";

    return true;
}


}
