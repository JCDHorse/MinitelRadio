//
// Created by justine on 20/12/25.
//

#include <HardwareSerial.h>

#include "WebRadioReceiver.h"

#include "CharQueue.h"
#include "mqtt.h"

static CharQueue awaiting_cmds;

const char *WebRadioReceiver::CHANNELS[CHANNEL_COUNT] = {
  "http://ice4.somafm.com/seventies-128-mp3",
  "http://stream.zenolive.com/507dzk77gkeuv",
  "http://streamer-03.rtl.fr/rtl-1-44-64?listen=webCwsBCggNCQgLDQUGBAcGBg;",
  "http://icecast.rtl.fr/rtl-1-44-64",
  "http://radios.rtbf.be/wr-c21-metal-128.mp3",
  "http://direct.franceinter.fr/live/franceinter-midfi.aac",
  "http://lyon1ere.ice.infomaniak.ch/lyon1ere-high.mp3",
  "http://stream.rcs.revma.com/5gd04cwptg0uv",
};

WebRadioReceiver::WebRadioReceiver(WiFiClient &_wifi_client)
: m_tone{0, 1, 0, 15}
{}

void msg_callback(const char * topic, const byte * payload, const unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    const char v = static_cast<char>(payload[i]);
    Serial.print(v);
    awaiting_cmds.push(v);
  }
  Serial.println();
}

void WebRadioReceiver::loop() {
  // if (!m_mqtt.connected()) {
  //   mqtt::reconnect(m_mqtt);
  // }
  // m_mqtt.loop();
  m_stream.loop();
  handle_command();
}

void WebRadioReceiver::handle_command() {
  char cmd = '\0';
  const bool has_cmd = awaiting_cmds.pop(cmd);

  if (!has_cmd) {
    return;
  }

  switch (cmd) {
  case 'n':
    next_channel();
    break;
  case 'v':
    prev_channel();
    break;
  case '+':
    volume_up();
    break;
  case '-':
    volume_down();
    break;
  case 'g':
    bass_up();
    break;
  case 'f':
    bass_down();
    break;
  case 'j':
    treble_up();
    break;
  case 'h':
    treble_down();
    break;
  case 'd':
    tone_default();
    break;
  case 's':
    // todo
    break;
  default:
    break;
  }
}

void WebRadioReceiver::init() {
  SPI.setHwCs(true);
  SPI.begin(SPI_CLK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN);

  // Initialize the VS1053 decoder
  if (!m_stream.startDecoder(VS1053_CS, VS1053_DCS, VS1053_DREQ)
      || !m_stream.isChipConnected()) {
    Serial.println("Decoder not running - system halted");
    return;
  }
  m_started = true;
  m_stream.setVolume(m_volume);
  connect_channel();
}

void WebRadioReceiver::connect_channel() {
  static char msg[MQTT_MSG_BUFFER_SIZE];

  Serial.print("Connection a ");

  m_stream.stopSong();
  Serial.print("Demande du stream: ");
  Serial.println(CHANNELS[m_channel]);

  const char* channel_url = CHANNELS[m_channel];

  snprintf(msg, MQTT_MSG_BUFFER_SIZE, "Changement de chaine radio: %s", channel_url);
  // mqtt::publish(m_mqtt, "webradio/inTopic", msg);
  m_stream.connecttohost(channel_url);

}

void WebRadioReceiver::next_channel() {
  m_channel = (m_channel + 1) % CHANNEL_COUNT;
  connect_channel();
}

void WebRadioReceiver::prev_channel() {
  m_channel = m_channel == 0 ? CHANNEL_COUNT - 1 : m_channel - 1;
  connect_channel();
}
void WebRadioReceiver::volume_up() {
  m_volume = constrain(m_volume + 5, 0, 100);
  m_stream.setVolume(m_volume);
}

void WebRadioReceiver::volume_down() {
  m_volume = constrain(m_volume - 5, 0, 100);
  m_stream.setVolume(m_volume);
}
void WebRadioReceiver::start() {
}

void WebRadioReceiver::stop() {
  m_stream.stopSong();
}

void WebRadioReceiver::bass_up() {
  m_tone[2]++;
  m_stream.setTone(m_tone);
}

void WebRadioReceiver::bass_down() {
  m_tone[2]--;
  m_stream.setTone(m_tone);
}

void WebRadioReceiver::treble_up() {
  m_tone[0]++;
  m_stream.setTone(m_tone);
}

void WebRadioReceiver::treble_down() {
  m_tone[0]--;
  m_stream.setTone(m_tone);
}

void WebRadioReceiver::tone_default() {
  m_tone[0] = 0;
  m_tone[1] = 1;
  m_tone[2] = 0;
  m_tone[3] = 15;
  m_stream.setTone(m_tone);
}

