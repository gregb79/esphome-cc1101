#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/spi/spi.h"
#include "esphome/components/number/number.h"
#include "esphome/components/select/select.h"
#include <vector>

namespace esphome {
namespace cc1101 {

class CC1101 
  : public sensor::Sensor,
    public PollingComponent,
    public spi::SPIDevice<spi::BIT_ORDER_MSB_FIRST, spi::CLOCK_POLARITY_LOW, spi::CLOCK_PHASE_LEADING, spi::DATA_RATE_1KHZ>
{
protected:
  InternalGPIOPin* gdo0_;
  InternalGPIOPin* gdo2_;
  uint32_t bandwidth_;
  uint32_t frequency_;
  sensor::Sensor* rssi_sensor_;
  sensor::Sensor* lqi_sensor_;

  uint8_t partnum_;
  uint8_t version_;
  int32_t last_rssi_;
  int32_t last_lqi_;

  bool reset();
  void send_cmd(uint8_t cmd);
  uint8_t read_register(uint8_t reg);
  uint8_t read_config_register(uint8_t reg);
  uint8_t read_status_register(uint8_t reg);
  void read_register_burst(uint8_t reg, uint8_t* buffer, size_t length);
  void write_register(uint8_t reg, uint8_t* value, size_t length);
  void write_register(uint8_t reg, uint8_t value);
  void write_register_burst(uint8_t reg, uint8_t* buffer, size_t length);

  // ELECHOUSE_CC1101 stuff

  bool mode_;
  uint8_t modulation_;
  float deviation_;
  uint8_t frend0_;
  uint8_t chan_;
  int8_t pa_;
  uint8_t last_pa_;
  uint8_t m4RxBw_;
  uint8_t m4DaRa_;
  uint8_t m2DCOFF_;
  uint8_t m2MODFM_;
  uint8_t m2MANCH_;
  uint8_t m2SYNCM_;
  uint8_t m1FEC_;
  uint8_t m1PRE_;
  uint8_t m1CHSP_;
  uint8_t trxstate_;
  uint8_t clb_[4][2];

  void set_mode(bool s);
  void set_frequency(uint32_t f);
  void set_modulation(uint8_t m);
  void set_deviation(float d);
  void set_pa(int8_t pa);
  void set_clb(uint8_t b, uint8_t s, uint8_t e);
  void set_rxbw(uint32_t bw);
  void set_tx();
  void set_rx();
  void set_sres();
  void set_sidle();
  void set_sleep();

  void split_MDMCFG2(); 
  void split_MDMCFG4();

public:
  CC1101();

  void set_config_gdo0(InternalGPIOPin* pin);
  void set_config_gdo2(InternalGPIOPin* pin);
  void set_config_bandwidth(uint32_t bandwidth);
  void set_config_frequency(uint32_t frequency);
  void set_config_modulation(int modulation);
  void set_config_deviation(float deviation);
  void set_config_rssi_sensor(sensor::Sensor* rssi_sensor);
  void set_config_lqi_sensor(sensor::Sensor* lqi_sensor);

  void setup() override;
  void update() override;
  void dump_config() override;

  int32_t get_rssi();
  uint8_t get_lqi();

  void begin_tx();
  void end_tx();

  std::vector<int> get_data(int id0, int id1, int instruction, int mode);

  void transmit_waveform(const std::vector<int> &waveform, uint8_t repeat);

  void set_spa_electric_id0_input(int value);
  void set_spa_electric_id1_input(int value);
  void set_spa_electric_instruction_input(int value);
  void set_spa_electric_mode_select(int value);

};

// Existing action templates
template<typename... Ts> class BeginTxAction : public Action<Ts...>, public Parented<CC1101>
{
public:
  void play(Ts... x) override { this->parent_->begin_tx(); }
};

template<typename... Ts> class EndTxAction : public Action<Ts...>, public Parented<CC1101>
{
public:
  void play(Ts... x) override { this->parent_->end_tx(); }
};

template<typename... Ts> class GetDataAction : public Action<Ts...>, public Parented<CC1101>
{
public:
  GetDataAction(number::Number *id0, number::Number *id1, number::Number *instruction, select::Select *mode)
    : id0_(id0), id1_(id1), instruction_(instruction), mode_(mode) {}

  void play(Ts... x) override {
    int mode_value;
    if (mode_->state == "Pool Only") {
      mode_value = 1;
    } else if (mode_->state == "Spa Only") {
      mode_value = 2;
    } else if (mode_->state == "Pool and Spa") {
      mode_value = 3;
    }
    this->parent_->get_data(
      (int)id0_->state,
      (int)id1_->state,
      (int)instruction_->state,
      mode_value
    );
  }

private:
  number::Number *id0_;
  number::Number *id1_;
  number::Number *instruction_;
  select::Select *mode_;
};

// New TransmitWaveformAction class
template<typename... Ts> class TransmitWaveformAction : public Action<Ts...>, public Parented<CC1101> {
public:
  TransmitWaveformAction(std::vector<int> waveform, uint8_t repeat)
    : waveform_(std::move(waveform)), repeat_(repeat) {}

  void play(Ts... x) override {
    this->parent_->transmit_waveform(waveform_, repeat_);
  }

private:
  std::vector<int> waveform_;
  uint8_t repeat_;
};

}  // namespace cc1101
}  // namespace esphome
