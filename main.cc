#include <iostream>
#include <glog/logging.h>
#include <hv/WebSocketServer.h>
#include <unordered_map>

#include "config.h"
#include "Connection.hpp"

void initWebSocketServer();
void initLogModule(bool);


int main(int argc, char** argv) {
    initLogModule(SAVE_LOGS_TO_FILE);

    google::InitGoogleLogging(argv[0]);

    LOG(INFO) << "Electron Buzzer Server, Developed by Haruki.";
    LOG(INFO) << "Current version: " << VERSION << ".";
    LOG(INFO) << "Listening: " << HOST << ":" << PORT << " ...";

    initWebSocketServer();

    return 0;
}

void initLogModule(bool flag) {
    FLAGS_colorlogtostdout = flag;
    FLAGS_logtostdout = flag;
//    FLAGS_log_dir = "../logs";        // debug
    FLAGS_log_dir = "./logs";           // release

}

void initWebSocketServer() {
    using namespace hv;
    WebSocketService webSocketService;
    std::unordered_map<WebSocketChannelPtr, uint32_t> connectedClients;
    MyConnection connections;

    webSocketService.onopen = [&connections](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
        LOG(INFO)
                << "[REQ] From: "
                << req->client_addr.ipport()
                << " | Path: "
                << req->Path();
        try {
//            char tmpStr[36];
//            uuid_t uuid;
//            uuid_generate(uuid);
//            uuid_unparse(uuid, tmpStr);
            connections.addChannel(channel);
            LOG(INFO) << "[REQ] Connection "<< channel->id() << " generated.";
        } catch (const std::string error) {
            LOG(ERROR) << error;
        }
    };

    webSocketService.onmessage = [&connections](const WebSocketChannelPtr& channel, const std::string& content) {
//        LOG(INFO)
//                << "[CONTENT] new content, length is: "
//                << content.length();
        connections.broadcast(content.data(), content.size(), channel);

    };

    webSocketService.onclose = [&connections](const WebSocketChannelPtr& channel) {
        LOG(INFO) << "[CLOSE] Connection " << channel->id() << " closed.";
        connections.removeChannel(channel);
    };

    WebSocketServer server;
    server.registerWebSocketService(&webSocketService);
    server.setHost(HOST);
    server.setPort(PORT);
    server.setThreadNum(4);
    LOG(INFO) << "WebSocket server is starting...";
    server.run();
}