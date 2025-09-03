#pragma once

#include <ELECHOUSE_CC1101_SRC_DRV.h>

#include "esphome/core/component.h"
#include <esphome/core/hal.h>

namespace esphome {
namespace cc1101 {

class CC1101 : public Component {

 public:
   CC1101(InternalGPIOPin *sck_pin, InternalGPIOPin *miso_pin, InternalGPIOPin *mosi_pin, InternalGPIOPin *csn_pin, InternalGPIOPin *gdo0_pin);
 
   void set_bandwidth(float bandwidth);
   void set_frequency(float frequency);
 
   void setup() override;
   void dump_config() override;

  protected:
  InternalGPIOPin *SCK_;
  InternalGPIOPin *MISO_;
  InternalGPIOPin *MOSI_;
  InternalGPIOPin *CSN_;
  InternalGPIOPin *GDO0_;

  float bandwidth_;
  float frequency_;
  
  float _moduleNumber;
};

}  // namespace cc1101
}  // namespace esphome
