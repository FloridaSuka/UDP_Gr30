#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <filesystem>
#include <chrono>
#include <thread>
#include <sstream>
#include <mutex>
#pragma comment(lib, "Ws2_32.lib")
namespace fs = std::filesystem;
using namespace std::chrono;
using namespace std;

const string SERVER_IP = "192.168.178.36";
const int SERVER_PORT = 8080;
const int MIN_CLIENTS = 2;
const int MAX_CLIENTS = 3;
const int TIMEOUT_SEC = 30;
const string DATA_DIR = "server_files";
const string STATS_FILE = "server_stats.txt";

struct Client {
    string ip;
    int port = 0;
    bool is_admin = false;
    int msg_count = 0;
    long long bytes_recv = 0;
    long long bytes_sent = 0;
    steady_clock::time_point last_active;
};

map<string, Client> clients;
mutex clients_mtx;
bool running = true;
SOCKET sockfd;

string addr_key(const sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &addr.sin_addr, ip, INET_ADDRSTRLEN);
    return string(ip);
}

void log_message(const string& ip, int port, const string& message) {
    ofstream log("chat_log.txt", ios::app);
    if (!log) return;
    auto now = system_clock::to_time_t(system_clock::now());
    char timebuf[100];
    ctime_s(timebuf, sizeof(timebuf), &now);
    string ts = timebuf;
    if (!ts.empty() && ts.back() == '\n') ts.pop_back();
    log << "[" << ts << "] " << ip << ":" << port << " -> " << message << "\n";
}
