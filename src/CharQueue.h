//
// Created by justine on 20/12/25.
//

#ifndef CHARQUEUE_H
#define CHARQUEUE_H

#include <cstdint>

#define QUEUE_SIZE 128

class CharQueue {
private:
  char m_buffer[QUEUE_SIZE] = { '\0' };
  uint8_t m_read = 0;
  uint8_t m_write = 0;
  uint8_t m_count = 0;
public:
  bool is_full() const;
  bool is_empty() const;

  bool push(char cmd);
  bool push_buffer(const char *data, size_t len);
  bool pop(char& out);
};



#endif // CHARQUEUE_H
