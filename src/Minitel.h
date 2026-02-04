//
// Created by justine on 25/01/26.
//

#ifndef MINITEL_H
#define MINITEL_H

#include <Minitel1B_Hard.h>  // Voir https://github.com/eserandour/Minitel1B_Hard

#define WEBRADIO_TITLE "3615 - WebRadio"

#define WIDTH 40
#define HEIGHT 25

enum RadioMode {
    INIT,
    WEB_RADIO,
};

class MinitelRadio {
public:
    MinitelRadio(const Minitel &minitel, QueueHandle_t &audio_cmd_queue, QueueHandle_t &audio_evt_queue);
    void new_page();
    void show_logline(String &log);
    void clear_logline();
    void show_wifi();
    void show_mode();
    void radio_page();
    void refresh();
    void handle_audio_events();
    void handle_keyboard();

    void set_wifi(const String& wifi_name);
    void set_mode(RadioMode mode);

private:
    Minitel m_minitel;
    int m_selection = 0;
    bool m_refresh = false;
    bool m_station_dirty = false;

    RadioMode m_mode = INIT;
    String m_wifi_name = "";
    String m_station_name;

    QueueHandle_t &m_audio_cmd_queue;
    QueueHandle_t &m_audio_evt_queue;

};



#endif //MINITEL_H
