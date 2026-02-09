#include "main.hpp"
#include "delay.h"
#include "RUR_STM_G4Lib/version.hpp"
#include "RUR_STM_G4Lib/Pin.hpp"
#include "RUR_STM_G4Lib/GPIO.hpp"
#include "RUR_STM_G4Lib/TIM.hpp"
#include "RUR_STM_G4Lib/UART.hpp"
#include "RUR_STM_G4Lib/SPI.hpp"
#include "RUR_STM_G4Lib/other.hpp"
#include "RUR_EtherCAT_Slave/soes_wrapper.hpp"
#include "EtherCAT/utypes.hpp"
#include "RUR_STM_G4Lib/OPAMP.hpp"
#include "RUR_STM_G4Lib/ADCv2.hpp"

#include "math.h"

G4::UART serial{G4::uart2, G4::PA2, G4::PA3};
G4::TIM tim1_1 = G4::TimerClockParameter::generate<G4::tim1, 40000>();
G4::TIM it_timer = G4::TimerClockParameter::generate<G4::tim2, static_cast<uint32_t>(1000)>();
G4::PWM comp_pwm1;
G4::PWM comp_pwm2;
G4::PWM comp_pwm3;

G4::OpAMP opamp1{G4::opamp1, G4::PA1};
G4::OpAMP opamp2{G4::opamp2, G4::PA7};
G4::OpAMP opamp3{G4::opamp3, G4::PB0};

// PB12 -> IN11, PB11 -> IN14
// ad_data[0]にIN11、ad_data[1]にIN14が入る
G4::ADCv2 ad1{G4::adc1, {G4::PB11, G4::PB12}};
G4::ADCv2 ad2{G4::adc2, {}};
std::vector<uint16_t> ad_data(2);
std::vector<uint16_t> ad_data2(1);

const float UVW_UNIT_VECTOR[3][2] = {
	{1.0f, 0.0f},
	{-0.5f, 0.86602540378f},
	{-0.5f, -0.86602540378f}
};

float feedback_current[3] = {0.0f};;
float offset_current[3] = {0.0f};

float duty_out = 0.1f;
float freq_out = 10.0f;

void timTask()
{
    static uint32_t count = 0;
    if (count++ > 1000) {
        count = 0;
        serial << feedback_current[0] << ", " << feedback_current[1] << "," << feedback_current[2] << "," << ad_data[0] << "," << ad_data[1] << "\n";
    }
    ad1.update();
}

void adc_task() {
    feedback_current[0] = -((ad1.getInjectionData(peripheral::adcv2::InjectionChannel::_1) - offset_current[0]) / 4095.0f * 3.3f / 2.0f);
    feedback_current[1] = -((ad2.getInjectionData(peripheral::adcv2::InjectionChannel::_1) - offset_current[1]) / 4095.0f * 3.3f / 2.0f);
    feedback_current[2] = -((ad2.getInjectionData(peripheral::adcv2::InjectionChannel::_2) - offset_current[2]) / 4095.0f * 3.3f / 2.0f);
}

void main_process(void)
{
    serial.config();

    G4::itTimerConfig(it_timer, 1, timTask);


    ad1.config({LL_ADC_CHANNEL_11, LL_ADC_CHANNEL_14}, {LL_ADC_CHANNEL_VOPAMP1}, ad_data, LL_ADC_SAMPLINGTIME_2CYCLES_5);
    ad2.config({}, {LL_ADC_CHANNEL_VOPAMP2, LL_ADC_CHANNEL_VOPAMP3_ADC2}, ad_data2, LL_ADC_SAMPLINGTIME_2CYCLES_5);

    ad1.configIT(adc_task);

    opamp1.config(peripheral::opamp::Mode::pga, peripheral::opamp::Gain::_2_or_minus_1);
    opamp2.config(peripheral::opamp::Mode::pga, peripheral::opamp::Gain::_2_or_minus_1);
    opamp3.config(peripheral::opamp::Mode::pga, peripheral::opamp::Gain::_2_or_minus_1);

    ad1.calibration();
    ad2.calibration();

    ad1.enable();
    ad2.enable();
    ad1.startConversion();
    ad2.startConversion();

    opamp1.enable();
    opamp2.enable();
    opamp3.enable();

    delay_ms(100);

    for(int i = 0; i < 10; i ++){
        offset_current[0] += ADC1->JDR1 / 10.0f;
        offset_current[1] += ADC2->JDR1 / 10.0f;
        offset_current[2] += ADC2->JDR2 / 10.0f;
        delay_ms(10);
    }

    serial << tim1_1.config(peripheral::tim::CounterMode::triangle, 1) << "\n";
    serial << tim1_1.configOC(G4::TIM::Ch::_1, G4::TIM::PWMMode::complementary) << "\n";
    serial << tim1_1.configOC(G4::TIM::Ch::_2, G4::TIM::PWMMode::complementary) << "\n";
    serial << tim1_1.configOC(G4::TIM::Ch::_3, G4::TIM::PWMMode::complementary) << "\n";
    serial << comp_pwm1.config(tim1_1.createChannel(G4::TIM::Ch::_1), G4::PA8, G4::PB13) << "\n";
    serial << comp_pwm2.config(tim1_1.createChannel(G4::TIM::Ch::_2), G4::PA9, G4::PB14) << "\n";
    serial << comp_pwm3.config(tim1_1.createChannel(G4::TIM::Ch::_3), G4::PA10, G4::PB15) << "\n";

    it_timer.setStart(true);
    tim1_1.setStart(true);
    comp_pwm1.setEnable(true);
    comp_pwm2.setEnable(true);
    comp_pwm3.setEnable(true);

    while (1)
    {
        comp_pwm1.setDuty(0.f);
        comp_pwm2.setDuty(40.f);
        comp_pwm3.setDuty(80.f);
    }
}
