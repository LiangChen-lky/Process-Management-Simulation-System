#include "message_queue.h"
#include <iostream>
#include <algorithm>

MessageQueue::MessageQueue() {}

bool MessageQueue::send(int from_pid, int to_pid, const std::string& content, int current_time) {
    messages_.emplace_back(from_pid, to_pid, content, current_time);
    return true;
}

bool MessageQueue::recv(int pid, Message& out_msg) {
    for (auto it = messages_.begin(); it != messages_.end(); ++it) {
        if (it->to_pid == pid) {
            out_msg = *it;
            messages_.erase(it);
            return true;
        }
    }
    return false;
}

bool MessageQueue::has_message(int pid) const {
    for (const auto& m : messages_) {
        if (m.to_pid == pid) return true;
    }
    return false;
}

std::vector<Message> MessageQueue::peek_all(int pid) const {
    std::vector<Message> result;
    for (const auto& m : messages_) {
        if (m.to_pid == pid) result.push_back(m);
    }
    return result;
}

void MessageQueue::clear(int pid) {
    messages_.erase(
        std::remove_if(messages_.begin(), messages_.end(),
            [pid](const Message& m) { return m.to_pid == pid || m.from_pid == pid; }),
        messages_.end());
}
