//
// Created by justine on 20/12/25.
//

#ifndef WEBRADIO_H
#define WEBRADIO_H

#include "ESP32_VS1053_Stream.h"
#include "VS1053.h"

#define SPI_CLK_PIN 5
#define SPI_MISO_PIN 19
#define SPI_MOSI_PIN 18

#define VS1053_CS 32
#define VS1053_DCS 33
#define VS1053_DREQ 15


class WebRadioReceiver {
private:
  static constexpr size_t CHANNEL_COUNT = 8;
  static const char *CHANNELS[CHANNEL_COUNT];

  ESP32_VS1053_Stream m_stream;

  bool m_started = false;
  size_t m_channel = 0;
  uint8_t m_volume = 70;
  uint8_t m_tone[4];
  uint8_t m_spatial = 0;

public:
  explicit WebRadioReceiver(WiFiClient &_wifi_client);
  void loop();
  void handle_command();
  void init();

  // Channels
  void connect_channel();
  void next_channel();
  void prev_channel();

  // Volume
  void volume_up();
  void volume_down();

  // Control
  void start();
  void stop();

  // Tone
  void bass_up();
  void bass_down();
  void treble_up();
  void treble_down();
  void tone_default();
};



#endif //WEBRADIO_H
