// client.cpp - NJË SOCKET, PA VONESA, PA PING TË VEÇANTË
#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <algorithm>
#pragma comment(lib, "Ws2_32.lib")
using namespace std;

const string SERVER_IP = "10.114.74.204";
const int SERVER_PORT = 8080;

std::string get_local_ip() {
    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s == INVALID_SOCKET) return "0.0.0.0";
    sockaddr_in dest = {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &dest.sin_addr);
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
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        cerr << "WSAStartup deshtoi!\n";
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Socket deshtoi!\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP.c_str(), &serv.sin_addr);

    sockaddr_in local{};
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = INADDR_ANY;
    local.sin_port = 0;
    if (bind(sock, (sockaddr*)&local, sizeof(local)) == SOCKET_ERROR) {
        cerr << "Bind deshtoi: " << WSAGetLastError() << endl;
        closesocket(sock); WSACleanup();
        return 1;
    }

    std::string ip = get_local_ip();
    bool is_admin = (ip == SERVER_IP);

    cout << "UDP SERVER - " << SERVER_IP << ":" << SERVER_PORT << endl;
    cout << "IP-ja jote: " << ip << " -> " << (is_admin ? "ADMIN" : "KLIENT i thjeshte") << endl;

    if (is_admin) {
        cout << "ADMIN: /list, /read, /stats, /search, /download, /upload, /delete, exit + chat\n";
    } else {
        cout << "KLIENT: /list, /read, exit + dërgo tekst\n";
    }
    cout << "> " << flush;

    // PING thread
    thread([&]() {
        while (true) {
            sendto(sock, "PING", 4, 0, (sockaddr*)&serv, sizeof(serv));
            Sleep(8000);
        }
    }).detach();

    string line;
    char buffer[131072];
    fd_set fds;
    timeval tv;

    // THREAD që dëgjon mesazhet spontane nga serveri
thread([&]() {
    char buf[4096];
    sockaddr_in from;
    int fromlen = sizeof(from);

    while (true) {
        int n = recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&from, &fromlen);
        if (n <= 0) continue;

        buf[n] = '\0';
        string msg(buf);

        // Injoro PONG
        if (msg == "PONG") continue;

        // MOS E FSHIJ — printo direkt
        cout << "\n" << msg << "\n> " << flush;
    }
}).detach();


    while (getline(cin, line)) {
        if (line == "exit") break;
        if (line.empty()) { cout << "> " << flush; continue; }

        if (!is_admin) {
            if (line == "/list" || line == "/read" || line.rfind("/read", 0) == 0) {}
            else if (line.rfind("/", 0) == 0) {
                cout << "Gabim: Komandë e palejuar.\n> " << flush;
                continue;
            }
        }

        sendto(sock, line.c_str(), (int)line.size(), 0, (sockaddr*)&serv, sizeof(serv));

        string full_response;
        bool received_something = false;
        timeval initial_tv = {2, 0};
        timeval short_tv = {0, 100000};

        while (true) {
            FD_ZERO(&fds);
            FD_SET(sock, &fds);
            timeval current_tv = received_something ? short_tv : initial_tv;

            if (select(0, &fds, nullptr, nullptr, &current_tv) <= 0) break;

            int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0, nullptr, nullptr);
            if (n <= 0) break;

            buffer[n] = '\0';
            string s(buffer);
            if (s == "PONG") continue;

            full_response += s;
            received_something = true;
        }

        if (!full_response.empty())
            cout << full_response << "\n> " << flush;
        else
            cout << "> " << flush;
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
