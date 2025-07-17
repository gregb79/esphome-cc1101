Skip to content
You said:
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
  std::vector<int> get_data(int id0, int id1, int instruction, int mode, int repeat);
  void set_spa_electric_id0_input(int value);
  void set_spa_electric_id1_input(int value);
  void set_spa_electric_instruction_input(int value);
  void set_spa_electric_mode_select(int value);
  void set_spa_electric_repeat_input(int value);
};

// BEGIN TX
template<typename... Ts>
class BeginTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->begin_tx(); }
};

// END TX
template<typename... Ts>
class EndTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->end_tx(); }
};

// GET DATA ACTION
template<typename... Ts>
class GetDataAction : public Action<Ts...>, public Parented<CC1101> {
public:
  GetDataAction() = default;

  void set_id0(number::Number *id0) { id0_ = id0; }
  void set_id1(number::Number *id1) { id1_ = id1; }
  void set_instruction(number::Number *instruction) { instruction_ = instruction; }
  void set_mode(select::Select *mode) { mode_ = mode; }
  void set_repeat(number::Number *repeat) { repeat_ = repeat; }

  void play(Ts... x) override {
    int mode_value = 0;
    if (mode_ != nullptr) {
      if (mode_->state == "Pool Only") {
        mode_value = 1;
      } else if (mode_->state == "Spa Only") {
        mode_value = 2;
      } else if (mode_->state == "Pool and Spa") {
        mode_value = 3;
      }
    }

    this->parent_->get_data(
      static_cast<int>(id0_ ? id0_->state : 0),
      static_cast<int>(id1_ ? id1_->state : 0),
      static_cast<int>(instruction_ ? instruction_->state : 0),
      mode_value,
      static_cast<int>(repeat_ ? repeat_->state : 1)
    );
  }

private:
  number::Number *id0_{nullptr};
  number::Number *id1_{nullptr};
  number::Number *instruction_{nullptr};
  select::Select *mode_{nullptr};
  number::Number *repeat_{nullptr};
};

}  // namespace cc1101
}  // namespace esphome . error compiling src/main.cpp: In function 'void setup()':
src/main.cpp:606:36: error: invalid conversion from 'int' to 'esphome::number::Number*' [-fpermissive]
   cc1101_getdataaction_id->set_id0(249);
                                    ^~~
In file included from src/esphome.h:23,
                 from src/main.cpp:3:
src/esphome\components\cc1101\cc1101.h:126:32: note:   initializing argument 1 of 'void esphome::cc1101::GetDataAction<Ts>::set_id0(esphome::number::Number*) [with Ts = {}]'
   void set_id0(number::Number *id0) { id0_ = id0; }
                ~~~~~~~~~~~~~~~~^~~
src/main.cpp:607:36: error: invalid conversion from 'int' to 'esphome::number::Number*' [-fpermissive]
   cc1101_getdataaction_id->set_id1(203);
                                    ^~~
In file included from src/esphome.h:23,
                 from src/main.cpp:3:
src/esphome\components\cc1101\cc1101.h:127:32: note:   initializing argument 1 of 'void esphome::cc1101::GetDataAction<Ts>::set_id1(esphome::number::Number*) [with Ts = {}]'
   void set_id1(number::Number *id1) { id1_ = id1; }
                ~~~~~~~~~~~~~~~~^~~
src/main.cpp:608:44: error: invalid conversion from 'int' to 'esphome::number::Number*' [-fpermissive]
   cc1101_getdataaction_id->set_instruction(17);
                                            ^~
In file included from src/esphome.h:23,
                 from src/main.cpp:3:
src/esphome\components\cc1101\cc1101.h:128:40: note:   initializing argument 1 of 'void esphome::cc1101::GetDataAction<Ts>::set_instruction(esphome::number::Number*) [with Ts = {}]'
   void set_instruction(number::Number *instruction) { instruction_ = instruction; }
                        ~~~~~~~~~~~~~~~~^~~~~~~~~~~
src/main.cpp:609:37: error: invalid conversion from 'int' to 'esphome::select::Select*' [-fpermissive]
   cc1101_getdataaction_id->set_mode(3);
                                     ^
In file included from src/esphome.h:23,
                 from src/main.cpp:3:
src/esphome\components\cc1101\cc1101.h:129:33: note:   initializing argument 1 of 'void esphome::cc1101::GetDataAction<Ts>::set_mode(esphome::select::Select*) [with Ts = {}]'
   void set_mode(select::Select *mode) { mode_ = mode; }
                 ~~~~~~~~~~~~~~~~^~~~
src/main.cpp:610:39: error: invalid conversion from 'int' to 'esphome::number::Number*' [-fpermissive]
   cc1101_getdataaction_id->set_repeat(5);
                                       ^
In file included from src/esphome.h:23,
                 from src/main.cpp:3:
src/esphome\components\cc1101\cc1101.h:130:35: note:   initializing argument 1 of 'void esphome::cc1101::GetDataAction<Ts>::set_repeat(esphome::number::Number*) [with Ts = {}]'
   void set_repeat(number::Number *repeat) { repeat_ = repeat; }
                   ~~~~~~~~~~~~~~~~^~~~~~
ChatGPT said:
The error you're encountering is caused by trying to pass integer literals (like 249, 203, 17, etc.) into functions that expect pointers to ESPHome components—specifically esphome::number::Number* or esphome::select::Select*.

Why This Fails
The GetDataAction class expects component pointers, not raw integer values:

void set_id0(number::Number *id0);
But you wrote:

cc1101_getdataaction_id->set_id0(249);  // <-- 249 is an int, not a Number*
This causes a type mismatch, and the compiler correctly complains.

How to Fix It
If you're trying to provide static/int values (like constants) to get_data(...) for one-time use, then you don’t need to use the component pointers (Number*, Select*)—you should create a simpler action or method that directly takes integers.

However, if you're trying to link to ESPHome number or select components, you need to do something like this in your YAML or C++ setup:

// Assuming you have these defined elsewhere and available:
auto id0_number = new number::Number();  // or fetched from ESPHome context
id0_number->state = 249;
cc1101_getdataaction_id->set_id0(id0_number);
But creating Number/Select instances manually like that is unusual and likely incorrect unless you're managing the entire ESPHome context.

Recommendation
You have two options:

✅ Option 1: Modify GetDataAction to allow raw integers
If you're just passing fixed values, change the class to accept raw values instead:

class GetDataAction : public Action<Ts...>, public Parented<CC1101> {
 public:
  void set_id0(int id0) { id0_ = id0; }
  void set_id1(int id1) { id1_ = id1; }
  void set_instruction(int instruction) { instruction_ = instruction; }
  void set_mode_value(int mode) { mode_ = mode; }
  void set_repeat(int repeat) { repeat_ = repeat; }

  void play(Ts... x) override {
    this->parent_->get_data(id0_, id1_, instruction_, mode_, repeat_);
  }

 protected:
  int id0_{0};
  int id1_{0};
  int instruction_{0};
  int mode_{0};
  int repeat_{1};
};
Then your original calls like this will work fine:

cc1101_getdataaction_id->set_id0(249);
cc1101_getdataaction_id->set_id1(203);
cc1101_getdataaction_id->set_instruction(17);
cc1101_getdataaction_id->set_mode_value(3);  // you'd have to change this name
cc1101_getdataaction_id->set_repeat(5);
🛠️ Option 2: Pass actual Number/Select components**
If you really need to reference dynamic ESPHome components:

number::Number *id0_component = ...;  // Get this from context
cc1101_getdataaction_id->set_id0(id0_component);
You’ll need to make sure these components are declared in your YAML or initialized in your C++ setup. This is harder unless you're deeply integrating into ESPHome’s internal systems.

Summary
You're trying to use set_id0(int) on a method that expects set_id0(Number*). To fix this, either refactor the class to accept int, or pass actual Number* components.

Let me know if you'd like help rewriting the class to use raw integers or to integrate with ESPHome's runtime component system.

You said:
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
  std::vector<int> get_data(int id0, int id1, int instruction, int mode, int repeat);
  void set_spa_electric_id0_input(int value);
  void set_spa_electric_id1_input(int value);
  void set_spa_electric_instruction_input(int value);
  void set_spa_electric_mode_select(int value);
  void set_spa_electric_repeat_input(int value);
};

// BEGIN TX
template<typename... Ts>
class BeginTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->begin_tx(); }
};

// END TX
template<typename... Ts>
class EndTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->end_tx(); }
};

// GET DATA ACTION
template<typename... Ts>
class GetDataAction : public Action<Ts...>, public Parented<CC1101> {
public:
  GetDataAction() = default;

  void set_id0(number::Number *id0) { id0_ = id0; }
  void set_id1(number::Number *id1) { id1_ = id1; }
  void set_instruction(number::Number *instruction) { instruction_ = instruction; }
  void set_mode(select::Select *mode) { mode_ = mode; }
  void set_repeat(number::Number *repeat) { repeat_ = repeat; }

  void play(Ts... x) override {
    int mode_value = 0;
    if (mode_ != nullptr) {
      if (mode_->state == "Pool Only") {
        mode_value = 1;
      } else if (mode_->state == "Spa Only") {
        mode_value = 2;
      } else if (mode_->state == "Pool and Spa") {
        mode_value = 3;
      }
    }

    this->parent_->get_data(
      static_cast<int>(id0_ ? id0_->state : 0),
      static_cast<int>(id1_ ? id1_->state : 0),
      static_cast<int>(instruction_ ? instruction_->state : 0),
      mode_value,
      static_cast<int>(repeat_ ? repeat_->state : 1)
    );
  }

private:
  number::Number *id0_{nullptr};
  number::Number *id1_{nullptr};
  number::Number *instruction_{nullptr};
  select::Select *mode_{nullptr};
  number::Number *repeat_{nullptr};
};

}  // namespace cc1101
}  // namespace esphome , show full modified code
ChatGPT said:
Here is the full modified version of your cc1101.h file where the GetDataAction class is updated to accept raw integer values instead of esphome::number::Number* and esphome::select::Select*. This allows you to pass values like 249, 203, etc., directly without needing to wrap them in ESPHome components.

✅ Modified cc1101.h (raw int support for GetDataAction)
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
  std::vector<int> get_data(int id0, int id1, int instruction, int mode, int repeat);
  void set_spa_electric_id0_input(int value);
  void set_spa_electric_id1_input(int value);
  void set_spa_electric_instruction_input(int value);
  void set_spa_electric_mode_select(int value);
  void set_spa_electric_repeat_input(int value);
};

// BEGIN TX
template<typename... Ts>
class BeginTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->begin_tx(); }
};

// END TX
template<typename... Ts>
class EndTxAction : public Action<Ts...>, public Parented<CC1101> {
public:
  void play(Ts... x) override { this->parent_->end_tx(); }
};

// GET DATA ACTION (Modified to accept raw int values)
template<typename... Ts>
class GetDataAction : public Action<Ts...>, public Parented<CC1101> {
public:
  GetDataAction() = default;

  void set_id0(int id0) { id0_ = id0; }
  void set_id1(int id1) { id1_ = id1; }
  void set_instruction(int instruction) { instruction_ = instruction; }
  void set_mode(int mode) { mode_ = mode; }
  void set_repeat(int repeat) { repeat_ = repeat; }

  void play(Ts... x) override {
    this->parent_->get_data(id0_, id1_, instruction_, mode_, repeat_);
  }

private:
  int id0_{0};
  int id1_{0};
  int instruction_{0};
  int mode_{0};
  int repeat_{1};
};

}  // namespace cc1101
}  // namespace esphome
✅ How to Use in main.cpp
Now you can use it like this without any compilation errors:

auto get_data_action = new esphome::cc1101::GetDataAction<>();
get_data_action->set_id0(249);
get_data_action->set_id1(203);
get_data_action->set_instruction(17);
get_data_action->set_mode(3);  // 3 = Pool and Spa (or whatever your logic dictates)
get_data_action->set_repeat(5);
Let me know if you want a version that supports both raw values and component references, or if you're also modifying the YAML integration.




Attach

Search

Voice
No file chosenNo file chosen
ChatGPT can make mistakes. Check important info.
