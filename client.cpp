// client.cpp - VERSIONI PËRFUNDIMTAR 100% ( /stats FUNKSIONON PERFECT + /download + çdo gjë )
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

const string SERVER_IP = "172.20.10.3";
const int    SERVER_PORT = 8080;

std::string get_local_ip() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) return "0.0.0.0";

    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &dest.sin_addr);

    // LIDHU ME SERVERIN TËND (jo me 8.8.8.8!)
    if (connect(s, (sockaddr*)&dest, sizeof(dest)) == SOCKET_ERROR) {
        closesocket(s);
        return "0.0.0.0";
    }

    sockaddr_in local = {};
    int len = sizeof(local);
    if (getsockname(s, (sockaddr*)&local, &len) == SOCKET_ERROR) {
        closesocket(s);
        return "0.0.0.0";
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &local.sin_addr, ip, INET_ADDRSTRLEN);
    closesocket(s);
    return std::string(ip);
}

int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2,2), &wsa);
    SOCKET cmd_sock = socket(AF_INET, SOCK_DGRAM, 0);
    SOCKET ping_sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serv.sin_addr);

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;
    bind(cmd_sock, (sockaddr*)&local, sizeof(local));

    std::string ip= get_local_ip();
    cout << "UDP SERVER - " << SERVER_IP << ":" << SERVER_PORT << endl;
    std::cout << "IP-ja jote: " << ip << " -> " 
              << (ip== SERVER_IP ? "ADMIN" : "KLIENT i thjeshte") << std::endl;
    cout << "Nese je ne server: je ADMIN! | /list , /read , /stats , /search , /download, /upload, /delete, exit \n";
    cout << "Nese je klient i thjeshte    | /list , /read , exit \n>";

    thread([&]() { while(true) { sendto(ping_sock, "PING", 4, 0, (sockaddr*)&serv, sizeof(serv)); Sleep(8000); } }).detach();

    string line;
    char buffer[131072];
    fd_set fds;
    timeval tv;

    while (getline(cin, line)) {
        if (line == "exit") break;
        if (line.empty()) continue;

        sendto(cmd_sock, line.c_str(), (int)line.size(), 0, (sockaddr*)&serv, sizeof(serv));

        // 1. NËSE ËSHTË /download → trajtim special binar me DOWNLOAD_START/END
        if (line.substr(0, 10) == "/download ") {
            string filename = line.substr(10);
            ofstream out(filename, ios::binary);
            if (!out) { cout << "Gabim: Nuk krijohet skedari.\n> "; continue; }

            long long total = 0;
            bool ended = false;

            while (!ended) {
                tv = {8, 0}; FD_ZERO(&fds); FD_SET(cmd_sock, &fds);
                if (select(0, &fds, nullptr, nullptr, &tv) <= 0) break;

                int n = recvfrom(cmd_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
                if (n <= 0) break;

                string pkt(buffer, n);
                if (pkt.find("DOWNLOAD_END") != string::npos) { ended = true; break; }

                if (pkt.find("DOWNLOAD_START|") == 0) {
                    size_t pos = pkt.find('\n');
                    if (pos != string::npos && pos + 1 < (size_t)n)
                        out.write(buffer + pos + 1, n - pos - 1);
                } else {
                    out.write(buffer, n);
                }
                total += n;
            }
            out.close();
            cout << "Shkarkuar: " << filename << " (" << total << " bytes)\n> ";
            continue;
        }

        // 2. PËR TË GJITHA KOMANDAT E TJERA (/list, /stats, /upload, etj.) → lexo TË GJITHA paketat
        string full_response;
        tv = {12, 0};  // kohë më e gjatë për /stats të madh

        while (true) {
            FD_ZERO(&fds); FD_SET(cmd_sock, &fds);
            timeval short_tv = {0, 100000};  // 0.1 sekondë
            if (select(0, &fds, nullptr, nullptr, &short_tv) <= 0) break;  // nuk ka më paketa

            int n = recvfrom(cmd_sock, buffer, sizeof(buffer)-1, 0, nullptr, nullptr);
            if (n <= 0) break;

            buffer[n] = '\0';
            string part(buffer);
            if (part != "PONG") full_response += part;
        }

        if (!full_response.empty()) {
            cout << full_response << "\n> ";
        } else {
            cout << "Asnje pergjigje nga serveri.\n> ";
        }

        // Pastro çdo mbetje përfundimtare
        timeval zero = {0,0};
        while (select(0, &fds, nullptr, nullptr, &zero) > 0) {
            recvfrom(cmd_sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
        }
    }

    closesocket(cmd_sock); closesocket(ping_sock); WSACleanup();
    return 0;
}