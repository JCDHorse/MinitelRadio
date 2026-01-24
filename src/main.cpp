#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard
#include <WiFiManager.h>

#include "WebRadioReceiver.h"

Minitel minitel(Serial2);

////////////////////////////////////////////////////////////////////////

#define TITRE "3615 - WebRadio"

String texte="";
int nbCaracteres=0;
const int PREMIERE_LIGNE_EXPRESSION = 4;
const int NB_LIGNES_EXPRESSION = 7;
const String VIDE = ".";

unsigned long touche;

WiFiClient wifi_client;
WiFiManager wifi_manager;

WebRadioReceiver wr_rcv(wifi_client);

////////////////////////////////////////////////////////////////////////

void newPage(String titre) {
  minitel.newScreen();
  minitel.println(titre);
  for (int i=1; i<=40; i++) {
    minitel.writeByte(0x7E);
  }
}
////////////////////////////////////////////////////////////////////////

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
  for(;;) {
    wr_rcv.loop();
    // ESSENTIEL : Donne du temps au processeur pour les tâches de fond
    vTaskDelay(1);
  }
}

////////////////////////////////////////////////////////////////////////

void setup() {
  Serial.begin(115200);
  minitel.changeSpeed(minitel.searchSpeed());
  minitel.smallMode();
  newPage(TITRE);
  Serial.println("\n\n3615 - WebRadio");
  Serial.println("");

  Serial.println("Controles: ");

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
  wr_rcv.init();

  // Priorité 5, on lance l'audio sur le core 0
  xTaskCreatePinnedToCore(audioTask, "AudioTask", 20000,
    NULL, 5, NULL, 0);
}


////////////////////////////////////////////////////////////////////////
///
///
///

bool main_menu_on = true;
int choice = 0;

void loop() {
// Affichage de la page
  newPage(TITRE);
  String wifi_name = wifi_manager.getWiFiSSID();
  show_wifi(wifi_name);
  choice = main_menu();

  Serial.println(choice);
}
