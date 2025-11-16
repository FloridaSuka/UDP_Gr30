None selected

Skip to content
Using University of Prishtina Mail with screen readers
Enable desktop notifications for University of Prishtina Mail.
   OK  No thanks

Conversations
 
Program Policies
Powered by Google
Last account activity: 1 hour ago
Details
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

const string SERVER_IP = "192.168.178.36";
const int SERVER_PORT = 8080;

void clear_socket_buffer(SOCKET sock) {
    u_long bytes_available = 0;
    if (ioctlsocket(sock, FIONREAD, &bytes_available) == 0 && bytes_available > 0) {
        char temp[131072];
        int to_read = (int)min(bytes_available, (u_long)sizeof(temp));
        while (bytes_available > 0) {
            int n = recvfrom(sock, temp, to_read, 0, nullptr, nullptr);
            if (n <= 0) break;
            bytes_available -= n;
            to_read = (int)min(bytes_available, (u_long)sizeof(temp));
        }
    }
}

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
        cout << "KLIENT: /list, /read, exit + dërgo tekst (p.sh. 'hello')\n";
    }
    cout << "> " << flush;

    // PING THREAD (në të njëjtin socket)
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

    while (getline(cin, line)) {
        if (line == "exit") break;
        if (line.empty()) { cout << "> " << flush; continue; }

        if (!is_admin) {
            if (line == "/list" || line == "/read" || line.rfind("/read ", 0) == 0) {
            } else if (line.rfind("/", 0) == 0) {
                cout << "Gabim: Komandë e palejuar. Përdor vetëm /list, /read, ose dërgo tekst.\n> " << flush;
                continue;
            }
        }

        sendto(sock, line.c_str(), (int)line.size(), 0, (sockaddr*)&serv, sizeof(serv));

        if (line.rfind("/", 0) != 0) {
            cout << "> " << flush;
            continue;
        }

        clear_socket_buffer(sock);

        if (is_admin && line.substr(0, 10) == "/download ") {
            string filename = line.substr(10);
            ofstream out(filename, ios::binary);
            if (!out) {
                cout << "Gabim: Nuk krijohet skedari.\n> " << flush;
                continue;
            }

            long long total = 0;
            bool ended = false;
            while (!ended) {
                tv = {8, 0}; FD_ZERO(&fds); FD_SET(sock, &fds);
                if (select(0, &fds, nullptr, nullptr, &tv) <= 0) break;
                int n = recvfrom(sock, buffer, sizeof(buffer), 0, nullptr, nullptr);
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
            cout << "Shkarkuar: " << filename << " (" << total << " bytes)\n> " << flush;
            continue;
        }

        string full_response;
        bool received_something = false;
        timeval initial_tv = {2, 0}; // 2 sekonda për përgjigjen e parë
        timeval short_tv = {0, 100000}; // 0.1 sekonda për paketa shtesë

        while (true) {
            FD_ZERO(&fds); FD_SET(sock, &fds);
            timeval current_tv = received_something ? short_tv : initial_tv;
            if (select(0, &fds, nullptr, nullptr, &current_tv) <= 0) break;
            int n = recvfrom(sock, buffer, sizeof(buffer)-1, 0, nullptr, nullptr);
            if (n <= 0) break;
            buffer[n] = '\0';
            string recv_str(buffer);
            if (recv_str == "PONG") {
                continue; // Injoroj PONG nga PING/PONG
            }
            full_response += recv_str;
            received_something = true;
        }

        if (!full_response.empty()) {
            cout << full_response << "\n> " << flush;
        } else {
            cout << "> " << flush;
        }

        clear_socket_buffer(sock);
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}
client.cpp
Displaying client.cpp. 