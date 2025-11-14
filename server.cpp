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

void stats_thread() {
    while (running) {
        Sleep(5000);
        lock_guard<mutex> lock(clients_mtx);
        ofstream f(STATS_FILE, ios::app);
        auto now = system_clock::to_time_t(system_clock::now());
        char timebuf[100];
        ctime_s(timebuf, sizeof(timebuf), &now);
        f << "\n=== STATS " << timebuf << "===\n";
        f << "Kliente aktive: " << clients.size() << " (min: " << MIN_CLIENTS << " | max: " << MAX_CLIENTS << ")\n";
        long long total_recv = 0, total_sent = 0;
        for (const auto& p : clients) {
            const Client& c = p.second;
            f << c.ip << ":" << c.port
                << " | Admin:" << (c.is_admin ? "PO" : "JO")
                << " | Msg:" << c.msg_count
                << " | Recv:" << c.bytes_recv << "B | Sent:" << c.bytes_sent << "B\n";
            total_recv += c.bytes_recv;
            total_sent += c.bytes_sent;
        }
        f << "TOTAL - Recv: " << total_recv << "B | Sent: " << total_sent << "B\n";
        f.close();
    }
}

void cleanup_thread() {
    while (running) {
        Sleep(5000);
        auto now = steady_clock::now();
        vector<string> to_remove;
        {
            lock_guard<mutex> lock(clients_mtx);
            for (const auto& p : clients) {
                if (duration_cast<seconds>(now - p.second.last_active).count() > TIMEOUT_SEC) {
                    to_remove.push_back(p.first);
                }
            }
            for (const auto& k : to_remove) {
                cout << "Timeout - Klienti u hoq: " << k << endl;
                clients.erase(k);
            }
        }
    }
}

string process_command(const string& cmdline, bool is_admin, const sockaddr_in& client_addr) {
    stringstream ss(cmdline);
    string cmd, arg;
    ss >> cmd;

    if (cmd == "/list") {
        stringstream out;
        for (const auto& e : fs::directory_iterator(DATA_DIR))
            out << e.path().filename().string() << "\n";
        return out.str().empty() ? "Nuk ka skedare.\n" : out.str();
    }
    if (cmd == "/read") {
        ss >> arg;
        ifstream f(DATA_DIR + "/" + arg);
        return f ? string((istreambuf_iterator<char>(f)), {}) + "\n" : "GABIM: Skedari nuk u gjet.\n";
    }

    if (!is_admin) return "Nuk ke leje per kete veprim.\n";
