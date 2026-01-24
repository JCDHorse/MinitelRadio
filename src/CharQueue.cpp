//
// Created by justine on 20/12/25.
//

#include <cstddef>
#include "CharQueue.h"

bool CharQueue::is_full() const {
  size_t e;
  return m_count == QUEUE_SIZE;
}

bool CharQueue::is_empty() const {
  return m_count == 0;
}

bool CharQueue::push(const char cmd) {
  if (is_full()) {
    return false;
  }
  m_buffer[m_write] = cmd;
  m_write = (m_write + 1) % QUEUE_SIZE;
  m_count++;
  return true;
}

bool CharQueue::push_buffer(const char* data, const size_t len) {
  for (size_t i = 0; i < len; ++i) {
    if (!push(data[i])) {
      return false; // overflow -> truncated command
    }
  }
  return true;
}

bool CharQueue::pop(char& out) {
  if (!out || is_empty()) {
    return false;
  }
  out = m_buffer[m_read];
  m_read = (m_read + 1) % QUEUE_SIZE;
  m_count--;
  return true;
}