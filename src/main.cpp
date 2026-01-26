#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard
#include <WiFiManager.h>

#include "Commands.h"
#include "Minitel.h"
#include "WebRadioReceiver.h"


////////////////////////////////////////////////////////////////////////

#define TITRE "3615 - WebRadio"

unsigned long touche;

MinitelUI minitel(Serial2);
WiFiClient wifi_client;
WiFiManager wifi_manager;

WebRadioReceiver wr_rcv(wifi_client);

// Queue des commandes audio
QueueHandle_t audio_cmd_queue;
// Queue des évenements audio
QueueHandle_t audio_evt_queue;

MinitelRadio *minitel_radio = nullptr;


#define MENU_ITEM_COUNT 2
const char *MENU_ITEMS[MENU_ITEM_COUNT] = {
  "Sélectionner la radio",
  "Écouter la radio",
};
int selection = 0;

void show_menu() {
//  newPage(TITRE);
  int x0 = 8, y0 = 10;

  for (int i = 0; i < MENU_ITEM_COUNT; i++) {
    minitel.moveCursorXY(x0, y0 + i);
    minitel.clearLineFromCursor();

    if (selection == i) {
      minitel.attributs(FOND_MAGENTA);
    } else {
      minitel.attributs(FOND_NORMAL);
    }
    minitel.print(String(i));
    minitel.print(". ");
    minitel.attributs(FOND_NORMAL);
    minitel.println(MENU_ITEMS[i]);
    minitel.clearLineFromCursor();
  }
  minitel.noCursor();
}

void show_wifi(String &wifi_name) {
  minitel.moveCursorXY(25, 0);
  minitel.clearLineFromCursor();
  minitel.print("WiFi: ");
  minitel.print(wifi_name);
}

int main_menu() {
  show_menu();
  bool selected = false;
  while (!selected) {
    touche = minitel.getKeyCode(true);
    if (touche == ENVOI) {
      selected = true;
    }
    else if (touche == 'j') {
      selection = (selection + 1) % MENU_ITEM_COUNT;
      show_menu();
    } else if (touche == 'k') {
      if (selection == 0) {
        selection = MENU_ITEM_COUNT - 1;
      }
      else {
        selection--;
      }
      show_menu();
    }
    // Permet de rendre la main au scheduler
    delay(1);
  }
  return selection;
}

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

////////////////////////////////////////////////////////////////////////

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


////////////////////////////////////////////////////////////////////////
///
///
///

bool main_menu_on = true;
int choice = 0;

void loop() {
  String wifi_name = wifi_manager.getWiFiSSID();
  minitel_radio->set_wifi(wifi_name);
  minitel_radio->set_mode(WEB_RADIO);
  minitel_radio->handle_audio_events();
  minitel_radio->radio_page();
}
