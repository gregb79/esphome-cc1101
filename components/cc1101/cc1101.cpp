/*
  https://github.com/gabest11/esphome-cc1101

  This is a CC1101 transceiver component that works with esphome's remote_transmitter/remote_receiver.
  
  It can be compiled with Arduino and esp-idf framework and should support any esphome compatible board through the SPI Bus.

  On ESP8266, you can use the same pin for GDO0 and GDO2 (it is an optional parameter).

  The source code is a mashup of the following github projects with some special esphome sauce:

  https://github.com/dbuezas/esphome-cc1101 (the original esphome component)
  https://github.com/nistvan86/esphome-q7rf (how to use esphome with spi)
  https://github.com/LSatan/SmartRC-CC1101-Driver-Lib (cc1101 setup code)

  TODO: RP2040? (USE_RP2040)
  TODO: Libretiny? (USE_LIBRETINY)
*/

#include "esphome/core/log.h"
#include "cc1101.h"
#include "cc1101defs.h"
#include <limits.h>
#include <sstream>
#include <vector>
#include <string>

#ifdef USE_ARDUINO
#include <Arduino.h>
#else // USE_ESP_IDF
#include <driver/gpio.h>
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#endif

namespace esphome {
namespace cc1101 {

static const char *TAG = "cc1101";

uint8_t PA_TABLE[8]     = {0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00};
//                       -30  -20  -15  -10   0    5    7    10
uint8_t PA_TABLE_315[8] = {0x12,0x0D,0x1C,0x34,0x51,0x85,0xCB,0xC2}; // 300 - 348 MHz
uint8_t PA_TABLE_433[8] = {0x12,0x0E,0x1D,0x34,0x60,0x84,0xC8,0xC0}; // 387 - 464 MHz
//                        -30  -20  -15  -10  -6    0    5    7    10   12
uint8_t PA_TABLE_868[10] = {0x03,0x17,0x1D,0x26,0x37,0x50,0x86,0xCD,0xC5,0xC0}; // 779 - 899.99 MHz
//                        -30  -20  -15  -10  -6    0    5    7    10   11
uint8_t PA_TABLE_915[10] = {0x03,0x0E,0x1E,0x27,0x38,0x8E,0x84,0xCC,0xC3,0xC0}; // 900 - 928 MHz

CC1101::CC1101() {
  this->gdo0_ = nullptr;
  this->gdo2_ = nullptr;
  this->bandwidth_ = 200;
  this->frequency_ = 433920; // Default 433.92 MHz
  this->rssi_sensor_ = nullptr;
  this->lqi_sensor_ = nullptr;

  this->partnum_ = 0;
  this->version_ = 0;
  this->last_rssi_ = INT_MIN;
  this->last_lqi_ = INT_MIN;

  this->mode_ = false;
  this->modulation_ = 2;  // Default ASK
  this->chan_ = 0;
  this->pa_ = 12;
  this->last_pa_ = -1;
  this->m4RxBw_ = 0;
  this->trxstate_ = 0;

  // Calibration values for different bands
  this->clb_[0][0] = 24; this->clb_[0][1] = 28;
  this->clb_[1][0] = 31; this->clb_[1][1] = 38;
  this->clb_[2][0] = 65; this->clb_[2][1] = 76;
  this->clb_[3][0] = 77; this->clb_[3][1] = 79;
}

void CC1101::set_config_gdo0(InternalGPIOPin* pin) {
  gdo0_ = pin;
  if(gdo2_ == nullptr) gdo2_ = pin;
}

void CC1101::set_config_gdo2(InternalGPIOPin* pin) {
  gdo2_ = pin;
}

void CC1101::set_config_bandwidth(uint32_t bandwidth) {
  bandwidth_ = bandwidth;
}

void CC1101::set_config_frequency(uint32_t frequency) {
  frequency_ = frequency;
}

void CC1101::set_config_modulation(int modulation) {
  modulation_ = modulation;
}

void CC1101::set_config_deviation(float deviation) {
  deviation_ = deviation;
}

void CC1101::set_config_rssi_sensor(sensor::Sensor* rssi_sensor) {
  rssi_sensor_ = rssi_sensor;
}

void CC1101::set_config_lqi_sensor(sensor::Sensor* lqi_sensor) {
  lqi_sensor_ = lqi_sensor;
}

void CC1101::setup() {
  if (gdo0_ == nullptr || gdo2_ == nullptr) {
    ESP_LOGE(TAG, "GDO0 or GDO2 pins are not configured");
    mark_failed();
    return;
  }

  this->gdo0_->setup();
  this->gdo2_->setup();
  this->gdo0_->pin_mode(gpio::FLAG_OUTPUT);
  this->gdo2_->pin_mode(gpio::FLAG_INPUT);

  this->spi_setup();

  if (!this->reset()) {
    mark_failed();
    ESP_LOGE(TAG, "Failed to reset CC1101 modem. Check connection.");
    return;
  }

  // Default initial setup registers - can be customized
  this->write_register(CC1101_FSCTRL1, 0x06);

  this->set_mode(false);
  this->set_frequency(this->frequency_);

  this->write_register(CC1101_MDMCFG1, 0x02);
  this->write_register(CC1101_MDMCFG0, 0xF8);
  this->write_register(CC1101_CHANNR, this->chan_);
  this->write_register(CC1101_DEVIATN, 0x47);
  this->write_register(CC1101_FREND1, 0x56);
  this->write_register(CC1101_MCSM0, 0x18);
  this->write_register(CC1101_FOCCFG, 0x16);
  this->write_register(CC1101_BSCFG, 0x1C);
  this->write_register(CC1101_AGCCTRL2, 0xC7);
  this->write_register(CC1101_AGCCTRL1, 0x00);
  this->write_register(CC1101_AGCCTRL0, 0xB2);
  this->write_register(CC1101_FSCAL3, 0xE9);
  this->write_register(CC1101_FSCAL2, 0x2A);
  this->write_register(CC1101_FSCAL1, 0x00);
  this->write_register(CC1101_FSCAL0, 0x1F);
  this->write_register(CC1101_FSTEST, 0x59);
  this->write_register(CC1101_TEST2, 0x81);
  this->write_register(CC1101_TEST1, 0x35);
  this->write_register(CC1101_TEST0, 0x09);
  this->write_register(CC1101_PKTCTRL1, 0x04);
  this->write_register(CC1101_ADDR, 0x00);
  this->write_register(CC1101_PKTLEN, 0x00);

  this->set_rxbw(this->bandwidth_);
  this->set_frequency(this->frequency_); // Ensure frequency is set correctly

  this->set_rx();

  ESP_LOGI(TAG, "CC1101 initialized.");
}

void CC1101::update() {
  if(this->rssi_sensor_ != nullptr) {
    int32_t rssi = this->get_rssi();
    if(rssi != this->last_rssi_) {
      this->rssi_sensor_->publish_state(rssi);
      this->last_rssi_ = rssi;
    }
  }

  if(this->lqi_sensor_ != nullptr) {
    int32_t lqi = this->get_lqi() & 0x7f; // MSB = CRC OK flag
    if(lqi != this->last_lqi_) {
      this->lqi_sensor_->publish_state(lqi);
      this->last_lqi_ = lqi;
    }
  }
}

void CC1101::set_mode(bool mode) {
  this->mode_ = mode;
  if (mode) {
    this->set_tx();
  } else {
    this->set_rx();
  }
}

void CC1101::set_frequency(uint32_t frequency_khz) {
  // frequency_khz in kHz, e.g. 433920 for 433.92 MHz

  // Determine the band and PA table
  if(frequency_khz < 400000) {
    memcpy(this->pa_, PA_TABLE_315, sizeof(PA_TABLE_315));
    this->band_ = 0;
  } else if(frequency_khz < 470000) {
    memcpy(this->pa_, PA_TABLE_433, sizeof(PA_TABLE_433));
    this->band_ = 1;
  } else if(frequency_khz < 930000) {
    memcpy(this->pa_, PA_TABLE_868, sizeof(PA_TABLE_868));
    this->band_ = 2;
  } else {
    memcpy(this->pa_, PA_TABLE_915, sizeof(PA_TABLE_915));
    this->band_ = 3;
  }

  // Calculate the frequency registers (24-bit freq word)
  uint32_t freq = (uint64_t)frequency_khz * (1 << 16) / 1000;  // freq word = freq (Hz) * 2^16 / 26MHz

  this->write_register(CC1101_FREQ2, (freq >> 16) & 0xFF);
  this->write_register(CC1101_FREQ1, (freq >> 8) & 0xFF);
  this->write_register(CC1101_FREQ0, freq & 0xFF);
}

void CC1101::set_rxbw(uint32_t bandwidth) {
  // Set RX bandwidth
  // Bandwidth must be between 58kHz and 812kHz

  if (bandwidth < 58) bandwidth = 58;
  else if (bandwidth > 812) bandwidth = 812;

  this->m4RxBw_ = (bandwidth >> 2) - 1;
  this->write_register(CC1101_MDMCFG4, (this->m4RxBw_ & 0xFF));
}

void CC1101::spi_setup() {
  // Implement SPI initialization for your platform here
  // This example assumes Arduino SPI with default settings

#ifdef USE_ARDUINO
  SPI.begin();
#endif
}

bool CC1101::reset() {
  // Send reset strobe command to CC1101

  this->spi_begin();
  this->spi_transfer(CC1101_SRES);
  this->spi_end();

  // Wait for chip to be ready after reset
  delay(100);

  // Read version number to verify communication
  this->version_ = this->read_register(CC1101_VERSION);
  this->partnum_ = this->read_register(CC1101_PARTNUM);

  if(this->version_ == 0 || this->version_ == 0xFF) {
    return false;  // No response from CC1101
  }
  return true;
}

uint8_t CC1101::read_register(uint8_t addr) {
  uint8_t val;
  this->spi_begin();
  this->spi_transfer(addr | 0x80);  // Read register command
  val = this->spi_transfer(0);
  this->spi_end();
  return val;
}

void CC1101::write_register(uint8_t addr, uint8_t val) {
  this->spi_begin();
  this->spi_transfer(addr);
  this->spi_transfer(val);
  this->spi_end();
}

void CC1101::spi_begin() {
#ifdef USE_ARDUINO
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(SS, LOW);
#endif
}

void CC1101::spi_end() {
#ifdef USE_ARDUINO
  digitalWrite(SS, HIGH);
  SPI.endTransaction();
#endif
}

uint8_t CC1101::spi_transfer(uint8_t data) {
#ifdef USE_ARDUINO
  return SPI.transfer(data);
#else
  return 0; // implement for esp-idf or other platforms
#endif
}

void CC1101::set_tx() {
  this->spi_begin();
  this->spi_transfer(CC1101_STX);
  this->spi_end();
  this->mode_ = true;
}

void CC1101::set_rx() {
  this->spi_begin();
  this->spi_transfer(CC1101_SRX);
  this->spi_end();
  this->mode_ = false;
}

int32_t CC1101::get_rssi() {
  int8_t rssi_dec = this->read_register(CC1101_RSSI);
  if (rssi_dec >= 128) rssi_dec = (rssi_dec - 256);
  return (rssi_dec / 2) - 74;
}

uint8_t CC1101::get_lqi() {
  return this->read_register(CC1101_LQI);
}

void CC1101::dump_config() {
  ESP_LOGCONFIG(TAG, "CC1101:");
  ESP_LOGCONFIG(TAG, "  Frequency: %u kHz", this->frequency_);
  ESP_LOGCONFIG(TAG, "  Bandwidth: %u kHz", this->bandwidth_);
  ESP_LOGCONFIG(TAG, "  Modulation: %u", this->modulation_);
  ESP_LOGCONFIG(TAG, "  Deviation: %f", this->deviation_);
  ESP_LOGCONFIG(TAG, "  GDO0 pin: %p", this->gdo0_);
  ESP_LOGCONFIG(TAG, "  GDO2 pin: %p", this->gdo2_);
}

}  // namespace cc1101
}  // namespace esphome
  ESP_LOGCONFIG(TAG, "  GDO2 pin: %p", this->gdo2_);
}

}  // namespace cc1101
}  // namespace esphome
