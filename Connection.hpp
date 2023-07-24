//
// Created by Haruki Kitahara on 7/24/23.
//
#include <hv/WebSocketServer.h>

class MyConnection {
public:
    void addChannel(const WebSocketChannelPtr& channel) {
        std::lock_guard<std::mutex> locker(mutex_);
        channels[channel->id()] = channel;
    }

    void removeChannel(const WebSocketChannelPtr& channel) {
        std::lock_guard<std::mutex> locker(mutex_);
        channels.erase(channel->id());
    }

    int foreachChannel(std::function<void(const WebSocketChannelPtr& channel)> fn) {
        std::lock_guard<std::mutex> locker(mutex_);
        for (auto& pair : channels) {
            fn(pair.second);
        }
        return channels.size();
    }

    int broadcast(const void* data, int size) {
        return foreachChannel([data, size](const WebSocketChannelPtr& channel) {
            channel->send((const char*)data, size);
        });
    }

    int broadcast(const std::string& str) {
        return broadcast(str.data(), str.size());
    }

    int broadcast(const void* data, int size, const WebSocketChannelPtr& ws) {
        return foreachChannel([data, size, ws](const WebSocketChannelPtr & channel) {
            if (ws->id() == channel->id())
                LOG(INFO) << "[SEND] Pass Connection " << channel->id();
            else {
                channel->send((const char *) data, size);
                LOG(INFO)
                        << "[CONTENT] content "
                        << size
                        << " content is sent to connection " << channel->id() << ".";
            }
        });
    }
private:
    std::map<int, WebSocketChannelPtr>  channels;
    std::mutex                          mutex_;
};
