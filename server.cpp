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
#include <queue>
#pragma comment(lib, "Ws2_32.lib")
namespace fs = std::filesystem;
using namespace std::chrono;
using namespace std;

queue<string> waiting_list;
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
    return string(ip) + ":" + to_string(ntohs(addr.sin_port));
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
            // Nëse ka klienta në pritje, aktivizo njërin
            while (clients.size() < MAX_CLIENTS && !waiting_list.empty()) {
                string next_key = waiting_list.front();
                waiting_list.pop();

                cout << "Aktivizim i klientit nga lista e pritjes: " << next_key << endl;

                // Rikrijo Client-in nga IP që kemi ruajtur si string "ip"
                sockaddr_in fake_addr{};
                fake_addr.sin_family = AF_INET;
                inet_pton(AF_INET, next_key.c_str(), &fake_addr.sin_addr);
                fake_addr.sin_port = 0; // do të përditësohet në mesazhin e radhës

                Client c;
                c.ip = next_key;
                c.port = 0;
                c.is_admin = (c.ip == SERVER_IP);
                c.last_active = steady_clock::now();
                clients[next_key] = c;
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

    if (cmd == "/download" && ss >> arg) {
        string path = DATA_DIR + "/" + arg;
        if (!fs::exists(path)) return "GABIM: Skedari nuk u gjet.\n";
        string header = "DOWNLOAD_START|" + arg + "|" + to_string(fs::file_size(path)) + "\n";
        sendto(sockfd, header.c_str(), header.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        ifstream file(path, ios::binary);
        char buf[65536];
        while (file.read(buf, sizeof(buf)) || file.gcount()) {
            sendto(sockfd, buf, file.gcount(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        }
        string end = "\nDOWNLOAD_END";
        sendto(sockfd, end.c_str(), end.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        return "";
    }
    if (cmd == "/upload" && ss >> arg) {
        string content = cmdline.substr(cmdline.find(arg) + arg.length() + 1);
        ofstream f(DATA_DIR + "/" + arg);
        return (f << content) ? "Ngarkuar me sukses.\n" : "Ngarkimi deshtoi.\n";
    }
    int main() {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            cerr << "WSAStartup deshtoi!\n";
            return 1;
        }
        fs::create_directories(DATA_DIR);
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        sockaddr_in serv{};
        serv.sin_family = AF_INET;
        serv.sin_port = htons(SERVER_PORT);
        inet_pton(AF_INET, SERVER_IP.c_str(), &serv.sin_addr);
        if (bind(sockfd, (sockaddr*)&serv, sizeof(serv)) == SOCKET_ERROR) {
            cerr << "Bind deshtoi: " << WSAGetLastError() << endl;
            return 1;
        }

        cout << "==================================================\n";
        cout << " UDP SERVER AKTIV - " << SERVER_IP << ":" << SERVER_PORT << endl;
        cout << " Admin = Vetem IP: " << SERVER_IP << "\n";
        cout << " Minimumi " << MIN_CLIENTS << " kliente, maksimumi " << MAX_CLIENTS << "\n";
        cout << "==================================================\n\n";

        thread(stats_thread).detach();
        thread(cleanup_thread).detach();

        char buffer[131072];
        sockaddr_in client_addr{};
        int addrlen = sizeof(client_addr);

        while (running) {
            int n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&client_addr, &addrlen);
            if (n <= 0) continue;
            buffer[n] = '\0';
            string request(buffer);
            string key = addr_key(client_addr);

            if (request != "PING") {
                log_message(inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), request);
            }

            Client* cl = nullptr;
            bool is_admin = false;
            int client_port = 0;

            {
                lock_guard<mutex> lock(clients_mtx);

                if (request == "PING") {
                    sendto(sockfd, "PONG", 4, 0, (sockaddr*)&client_addr, addrlen);
                    continue;
                }

                if (clients.find(key) == clients.end()) {

                    // Ka vend? Fut klientin brenda
                    if (clients.size() < MAX_CLIENTS) {
                        Client c;
                        c.ip = inet_ntoa(client_addr.sin_addr);
                        c.port = ntohs(client_addr.sin_port);
                        c.is_admin = (c.ip == SERVER_IP);
                        c.last_active = steady_clock::now();
                        clients[key] = c;

                        cout << "Lidhur: " << c.ip << " [PORT:" << c.port << "]"
                            << (c.is_admin ? " [ADMIN]" : " [KLIENT]")
                            << " | Total: " << clients.size() << "/" << MAX_CLIENTS << endl;
                    }
                    else {
                        // Nuk ka vend ? shtoje klientin në pritje
                        waiting_list.push(key);
                        string msg = "Ne pritje: Serveri eshte i mbushur. Ju lutem prisni...\n";
                        sendto(sockfd, msg.c_str(), msg.size(), 0, (sockaddr*)&client_addr, addrlen);
                        continue;
                    }
                }

                cl = &clients[key];
                cl->port = ntohs(client_addr.sin_port);  
                if (request != "PING") cl->last_active = steady_clock::now();
                cl->msg_count++;
                cl->bytes_recv += n;

                is_admin = cl->is_admin;
                client_port = cl->port;

                if (clients.size() < MIN_CLIENTS && request != "PING") {
                    string msg = "Ne pritje: Duhet minimumi " + to_string(MIN_CLIENTS) + " kliente (tani: " + to_string(clients.size()) + ")\n";
                    sendto(sockfd, msg.c_str(), msg.size(), 0, (sockaddr*)&client_addr, addrlen);
                    continue;
                }
            }

            if (!cl) continue;

            string response;
            if (request.rfind("/", 0) != 0) {
                cout << "MESAZH I MARRË: [" << cl->ip << ":" << client_port << "] " << request << endl;
                response = "";
            }
            else {
                if (!is_admin) {
                    Sleep(40);
                }
                response = process_command(request, is_admin, client_addr);
            }

            if (!response.empty()) {
                lock_guard<mutex> lock(clients_mtx);
                cl->bytes_sent += response.size();

                sockaddr_in reply_addr = client_addr;
                reply_addr.sin_port = htons(cl->port);

                sendto(sockfd, response.c_str(), response.size(), 0, (sockaddr*)&reply_addr, sizeof(reply_addr));
            }
        }

                closesocket(sockfd);
                WSACleanup();
                return 0;
            }

