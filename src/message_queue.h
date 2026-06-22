#pragma once
#include "pcb.h"
#include <queue>
#include <vector>
#include <string>

class MessageQueue {
public:
    MessageQueue();

    bool send(int from_pid, int to_pid, const std::string& content, int current_time);
    bool recv(int pid, Message& out_msg);
    bool has_message(int pid) const;
    std::vector<Message> peek_all(int pid) const;
    void clear(int pid);

private:
    std::vector<Message> messages_;
};
