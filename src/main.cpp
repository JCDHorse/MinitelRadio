#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard
#include <WiFiManager.h>

#include "Commands.h"
#include "Minitel.h"
#include "WebRadioReceiver.h"

#define TITRE "3615 - WebRadio"

unsigned long touche;

Minitel minitel(Serial2);
WiFiClient wifi_client;
WiFiManager wifi_manager;

WebRadioReceiver wr_rcv(wifi_client);

// Queue des commandes audio
QueueHandle_t audio_cmd_queue;
// Queue des évenements audio
QueueHandle_t audio_evt_queue;

MinitelRadio *minitel_radio = nullptr;

void audioTask(void* data) {
  vTaskDelay(2000 / portTICK_PERIOD_MS);
  while(WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.println("AudioTask: Still waiting for WiFi...");
  }
  // Laisse le temps a tout les systèmes du réseau de bien se lancer
  vTaskDelay(2000 / portTICK_PERIOD_MS);

  Command audio_cmd;

  for(;;) {
    wr_rcv.loop();

    if (xQueueReceive(audio_cmd_queue, &audio_cmd, 0) == pdTRUE) {
      switch (audio_cmd.type) {
      case CMD_AUDIO_NEXT_RADIO:
        wr_rcv.next_channel();
        break;
      case CMD_AUDIO_PREV_RADIO:
        wr_rcv.prev_channel();
        break;
      case CMD_AUDIO_VOLUME_UP:
        wr_rcv.volume_up();
        break;
      case CMD_AUDIO_VOLUME_DOWN:
        wr_rcv.volume_down();
        break;
      default:
        break;
      }

    }

    vTaskDelay(1);
  }
}


void setup() {
  Serial.begin(115200);
  int speed = minitel.searchSpeed();
  minitel.changeSpeed(speed);
  minitel.smallMode();
  Serial.println("\n\n3615 - WebRadio");
  Serial.println("");

  Serial.println("Controles: ");

  minitel.newScreen();

  minitel.attributs(DOUBLE_GRANDEUR);
  minitel.println("3615 WEBRADIO");
  minitel.attributs(GRANDEUR_NORMALE);
  minitel.bip();
  delay(100);
  minitel.print("Connexion au minitel a ");
  minitel.print(String(speed));
  minitel.println(" bauds.");


  Serial.print("Connexion au reseau ");
  minitel.println("Connexion au réseau...");

  const bool res = wifi_manager.autoConnect("Justine-ESP32");

  if (!res) {
    Serial.println("Failed to connect");
    minitel.println("Échec de la connexion.");
    minitel.println("Redémarrage du contrôleur...");
    delay(2000);
    ESP.restart();
  }

  Serial.println("WiFi connected");
  minitel.println("Wifi connecté !");
  minitel.print("SSID: ");
  minitel.println(wifi_manager.getWiFiSSID());
  Serial.println("Adresse IP: ");
  minitel.print("IP: ");
  Serial.println(WiFi.localIP());
  minitel.println(WiFi.localIP().toString());

  Serial.println("Initialisation de l'audio...");
  minitel.println("Initialisation de l'audio...");
  minitel.println("Audio initialisé !");
  minitel.bip();
  delay(1000);

  // Initialisation des queues
  audio_cmd_queue = xQueueCreate(10, sizeof(Command));
  audio_evt_queue = xQueueCreate(5, sizeof(AudioEvent));

  wr_rcv.set_evt_queue(audio_evt_queue);
  wr_rcv.init();

  // Priorité 5, on lance l'audio sur le core 0
  xTaskCreatePinnedToCore(audioTask, "AudioTask", 20000,
    NULL, 5, NULL, 0);

  minitel_radio = new MinitelRadio(minitel, audio_cmd_queue, audio_evt_queue);
  minitel_radio->refresh();
}

void loop() {
  String wifi_name = wifi_manager.getWiFiSSID();
  minitel_radio->set_wifi(wifi_name);
  minitel_radio->set_mode(WEB_RADIO);
  minitel_radio->handle_audio_events();
  minitel_radio->radio_page();
  minitel_radio->handle_keyboard();
}
