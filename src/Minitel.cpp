//
// Created by justine on 25/01/26.
//

#include <Minitel1B_Hard.h>

#include "Minitel.h"


#include "Commands.h"

static constexpr size_t MENU_ITEMS_COUNT = 2;
static const char *MENU_ITEMS[MENU_ITEMS_COUNT] = {
    "Sélectionner la radio",
    "Écouter la radio",
  };


MinitelRadio::MinitelRadio(const Minitel &minitel,
    QueueHandle_t &audio_cmd_queue, QueueHandle_t &audio_evt_queue)
    : m_minitel(minitel), m_audio_cmd_queue(audio_cmd_queue), m_audio_evt_queue(audio_evt_queue)
{
    m_minitel.changeSpeed(m_minitel.searchSpeed());
    m_minitel.echo(false);
    m_minitel.smallMode();
}

void MinitelRadio::new_page()
{
    m_minitel.newScreen();
    m_minitel.println(WEBRADIO_TITLE);
    show_wifi();
    show_mode();
    m_minitel.hLine(0, 2, WIDTH - 1, CENTER);
}

void MinitelRadio::show_logline(String &log) {
    m_minitel.moveCursorXY(0, 12);
    m_minitel.println(log);
    m_minitel.clearLineFromCursor();
}

void MinitelRadio::clear_logline() {
    m_minitel.moveCursorXY(0, 12);
    m_minitel.clearLineFromCursor();
}

void MinitelRadio::show_wifi()
{
    m_minitel.moveCursorXY(25, 0);
    m_minitel.clearLineFromCursor();
    m_minitel.print("WiFi: ");
    m_minitel.print(m_wifi_name);
}

void MinitelRadio::show_mode()
{
    String mode_str;
    switch (m_mode)
    {
    case INIT:
        mode_str = "Init";
        break;
    case WEB_RADIO:
        mode_str = "WebRadio";
        break;
    default:
        mode_str = "???";
        break;
    }
    m_minitel.moveCursorXY(0, HEIGHT - 1);
    m_minitel.clearLineFromCursor();
    m_minitel.print("Mode: ");
    m_minitel.println(mode_str);
}

void MinitelRadio::radio_page()
{
    if (m_refresh) {
        new_page();
        constexpr int x0 = 10, y0 = 5;
        constexpr int width = 20, height = 5;
        m_minitel.vLine(x0, y0, y0 + height, LEFT, DOWN);
        m_minitel.vLine(x0 + width, y0, y0 + height, RIGHT, DOWN);
        m_minitel.hLine(x0, y0, x0 + width, TOP);
        m_minitel.hLine(x0, y0 + height, x0 + width, TOP);
        m_minitel.moveCursorXY(x0 + 1, y0 + 1);
        m_minitel.println("Vous écoutez : ");
        m_minitel.moveCursorXY(x0 + 1, y0 + 3);
        m_minitel.print("     ");
        m_minitel.println(m_station_name);

        m_minitel.moveCursorXY((WIDTH / 2), HEIGHT - 2);
        m_minitel.print("Vol      Chaine");
        m_minitel.moveCursorXY((WIDTH / 2), HEIGHT - 1);
        m_minitel.print("A/Q    SUITE/RETOUR");
        m_refresh = false;
    }

    if (m_station_dirty) {
        constexpr int x = 11;
        constexpr int y = 8;

        m_minitel.moveCursorXY(x, y);
        m_minitel.clearLineFromCursor();
        m_minitel.print(m_station_name);

        m_station_dirty = false;
        clear_logline();
    }

}

void MinitelRadio::refresh()
{
    m_refresh = true;
}

void MinitelRadio::handle_audio_events()
{
    AudioEvent evt = {};

    while (xQueueReceive(m_audio_evt_queue, &evt, 0) == pdTRUE) {
        switch (evt.type) {
        case EVT_STATION_NAME:
            m_station_name = evt.text;
            m_station_dirty = true;
            break;
        }
    }
}

void MinitelRadio::handle_keyboard() {
    String logline = "";
    Command cmd = {};
    const unsigned long touch = m_minitel.getKeyCode(true);
    switch (touch)
    {
        case SUITE:
            cmd.type = CMD_AUDIO_NEXT_RADIO;
            logline = "Station suivante...";
            cmd.value = 1;
            break;
        case RETOUR:
            cmd.type = CMD_AUDIO_PREV_RADIO;
            logline = "Station précédente...";
            cmd.value = -1;
            break;
        case 'A':
        case 'a':
            cmd.type = CMD_AUDIO_VOLUME_UP;
            cmd.value = +1;
            break;
        case 'Q':
        case 'q':
            cmd.type = CMD_AUDIO_VOLUME_DOWN;
            cmd.value = -1;
            break;
        default:
            return;
    }
    if (!logline.isEmpty()) {
        show_logline(logline);
    }
    xQueueSend(m_audio_cmd_queue, &cmd, 0);
}

void MinitelRadio::set_wifi(const String& wifi_name)
{
    m_wifi_name = wifi_name;
}

void MinitelRadio::set_mode(RadioMode mode)
{
    m_mode = mode;
}
