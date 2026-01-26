//
// Created by justine on 25/01/26.
//

#ifndef COMMANDS_H
#define COMMANDS_H
#include <cstdint>

#define COMMAND_PAYLOAD_SIZE 128

enum CommandType {
    CMD_AUDIO_PREV_RADIO,
    CMD_AUDIO_NEXT_RADIO,
    CMD_AUDIO_PLAY_URL,
    CMD_AUDIO_STOP,
    CMD_AUDIO_VOLUME_UP,
    CMD_AUDIO_VOLUME_DOWN,
};

struct Command {
    CommandType type;
    int value;
};

enum AudioEventType
{
    EVT_STATION_NAME,
};

struct AudioEvent {
    char text[64];
    AudioEventType type;
};

#endif //COMMANDS_H
