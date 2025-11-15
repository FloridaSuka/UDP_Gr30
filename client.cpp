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