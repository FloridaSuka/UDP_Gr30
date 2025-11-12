#define _WIN32_WINNT 0x0600
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <filesystem>
#include <iomanip>
#include <vector>
#pragma comment(lib, "ws2_32.lib")
using namespace std;

namespace fs = std::filesystem;
using namespace std::chrono;

const int PORT = 54000;
const sstring SERVER_IP = "0.0.0.0";
const string ADMIN_IP = "192.168.178.36";
const string DATA_DIR = "server_data";
const string LOG_FILE = "udp_server_stats.txt";
const int TIMEOUT_SEC = 30;
const int PACKET_SIZE = 1400;  

struct ClientInfo {
    string ip;
    int port;
    int messages = 0;
    long long bytes_in = 0;
    long long bytes_out = 0;
    steady_clock::time_point last_seen;
    bool is_admin = false;
    unordered_map<uint32_t, vector<char>> pending_file; 
    string current_file = "";
    uint32_t expected_seq = 0;
};