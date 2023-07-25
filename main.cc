#include <glog/logging.h>
#include <hv/hlog.h>
#include <hv/WebSocketServer.h>
#include <yaml-cpp/yaml.h>
#include <unordered_map>
#include <fstream>

#include "config.h"
#include "Connection.hpp"

YAML::Node config;

void configuration();
void initWebSocketServer();
void initLogModule(bool, char**);


int main(int argc, char** argv) {
    configuration();
    initLogModule(config["log"].as<bool>(), argv);

    LOG(INFO) << "Electron Buzzer Server, Developed by Haruki.";
    LOG(INFO) << "Current version: " << VERSION << ".";
    LOG(INFO) << "Listening: " << config["host"].as<std::string>()
              << ":" << config["port"].as<std::string>() << " ...";

    initWebSocketServer();

    google::ShutdownGoogleLogging();
    return 0;
}

void configuration() {
    assert(config.IsNull());
    namespace fs = std::filesystem;
    fs::path configPath = "./config.yaml";
    if (fs::exists(configPath)) {
        std::ifstream file(configPath);
        config = YAML::Load(file);
        std::cout << "File config.yaml is found, loading..." << '\n';
    } else {
        std::cerr << "File config.yaml is not found."
            << "\n" << "Using default settings..." << '\n';
        YAML::Node tmpNode;
        assert(tmpNode.IsNull());
        tmpNode["host"] = "0.0.0.0";
        tmpNode["port"] = 8765;
        tmpNode["log"] = "true";

        std::ofstream file;
        file.open(configPath, std::ios_base::out);
        if (!file.is_open()) {
            std::cerr << "Failed to open " << configPath << '\n';
            exit(-1);
        } else {
            file << tmpNode << std::endl;
        }
        config = tmpNode;
    }
}

void initLogModule(bool flag, char** argv) {
    namespace fs = std::filesystem;
    const fs::path& logs_dir = "./logs";

    if (!fs::exists(logs_dir))
        fs::create_directory(logs_dir);

    google::InitGoogleLogging(argv[0]);
    FLAGS_log_dir = logs_dir.string().c_str();
    logger_set_level(hlog, LOG_LEVEL_SILENT);
    FLAGS_colorlogtostdout = flag;
//    FLAGS_logtostdout = !flag;
    FLAGS_alsologtostderr = flag;
    FLAGS_max_log_size = 10;            // 10MB

}

void initWebSocketServer() {
    using namespace hv;
    WebSocketService webSocketService;
    std::unordered_map<WebSocketChannelPtr, uint32_t> connectedClients;
    MyConnection connections;
    webSocketService.ping_interval = 0;
    webSocketService.onopen = [&connections](const WebSocketChannelPtr& channel, const HttpRequestPtr& req) {
        LOG(INFO)
                << "[REQ] From: "
                << req->client_addr.ipport()
                << " | Path: "
                << req->Path();
        try {
            connections.addChannel(channel);
            LOG(INFO) << "[REQ] Connection "<< channel->id() << " generated.";
        } catch (const std::string error) {
            LOG(ERROR) << error;
        }
    };

    webSocketService.onmessage = [&connections](const WebSocketChannelPtr& channel, const std::string& content) {
        connections.broadcast(content.data(), content.size(), channel);

    };

    webSocketService.onclose = [&connections](const WebSocketChannelPtr& channel) {
        LOG(INFO) << "[CLOSE] Connection " << channel->id() << " closed.";
        connections.removeChannel(channel);
    };

    WebSocketServer server;
    server.registerWebSocketService(&webSocketService);
    server.setHost(config["host"].as<std::string>().c_str());
    server.setPort(config["port"].as<int>());
    server.setThreadNum(4);
    LOG(INFO) << "WebSocket server is starting...";
    server.run();
}