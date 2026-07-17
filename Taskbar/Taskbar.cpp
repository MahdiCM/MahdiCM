#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <windows.h>
#include <iphlpapi.h> 

#pragma comment(lib, "iphlpapi.lib")

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string BLUE = "\033[34m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string RED = "\033[31m";
const std::string MAGENTA = "\033[35m";

void setCursor(int x, int y) {
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD dwPos = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(hCon, dwPos);
}

void hideCursor() {
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hCon, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hCon, &cursorInfo);
}

double getCPUUsage() {
    static FILETIME idleTimeO, kernelTimeO, userTimeO;
    FILETIME idleTime, kernelTime, userTime;
    if (!GetSystemTimes(&idleTime, &kernelTime, &userTime)) return 0.0;

    ULONGLONG idleO = ((ULONGLONG)idleTimeO.dwHighDateTime << 32) | idleTimeO.dwLowDateTime;
    ULONGLONG kernelO = ((ULONGLONG)kernelTimeO.dwHighDateTime << 32) | kernelTimeO.dwLowDateTime;
    ULONGLONG userO = ((ULONGLONG)userTimeO.dwHighDateTime << 32) | userTimeO.dwLowDateTime;

    ULONGLONG idle = ((ULONGLONG)idleTime.dwHighDateTime << 32) | idleTime.dwLowDateTime;
    ULONGLONG kernel = ((ULONGLONG)kernelTime.dwHighDateTime << 32) | kernelTime.dwLowDateTime;
    ULONGLONG user = ((ULONGLONG)userTime.dwHighDateTime << 32) | userTime.dwLowDateTime;

    ULONGLONG dIdle = idle - idleO;
    ULONGLONG dKernel = kernel - kernelO;
    ULONGLONG dUser = user - userO;
    ULONGLONG dSystem = dKernel + dUser;

    idleTimeO = idleTime; kernelTimeO = kernelTime; userTimeO = userTime;
    if (dSystem == 0) return 0.0;
    return ((double)(dSystem - dIdle) / dSystem) * 100.0;
}

std::string formatBytes(double bytes) {
    const std::string suffix[] = { "B", "KB", "MB", "GB" };
    int i = 0;
    while (bytes >= 1024 && i < 3) {
        bytes /= 1024;
        i++;
    }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << bytes << " " << suffix[i];
    return ss.str();
}

void getNetworkBytes(ULONG64& outInBytes, ULONG64& outOutBytes) {
    outInBytes = 0;
    outOutBytes = 0;
    DWORD dwSize = 0;

    if (GetIfTable(NULL, &dwSize, FALSE) == ERROR_INSUFFICIENT_BUFFER) {
        MIB_IFTABLE* pIfTable = (MIB_IFTABLE*)malloc(dwSize);
        if (pIfTable != NULL) {
            if (GetIfTable(pIfTable, &dwSize, FALSE) == NO_ERROR) {
                for (DWORD i = 0; i < pIfTable->dwNumEntries; i++) {
                    if (pIfTable->table[i].dwType == MIB_IF_TYPE_ETHERNET ||
                        pIfTable->table[i].dwType == 71 ) {

                        outInBytes += pIfTable->table[i].dwInOctets;
                        outOutBytes += pIfTable->table[i].dwOutOctets;
                    }
                }
            }
            free(pIfTable); 
        }
    }
}

void drawProgressBar(double percentage, int width) {
    int pos = static_cast<int>(width * (percentage / 100.0));
    std::string barColor = GREEN;
    if (percentage > 80.0) barColor = RED;
    else if (percentage > 50.0) barColor = YELLOW;

    std::cout << BOLD << CYAN << "[" << barColor;
    for (int i = 0; i < width; ++i) {
        if (i < pos) std::cout << "■";
        else std::cout << " ";
    }
    std::cout << CYAN << "] " << barColor << std::fixed << std::setprecision(1) << std::setw(5) << percentage << "%" << RESET;
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    hideCursor();

    const std::vector<std::string> radarFrames = {
        "  ⚡ [ 📡  LIVE SYSTEM PROTOCOLS .      ] ⚡",
        "  ⚡ [ 📡  LIVE SYSTEM PROTOCOLS ..     ] ⚡",
        "  ⚡ [ 📡  LIVE SYSTEM PROTOCOLS ...    ] ⚡",
        "  ⚡ [ 📡  LIVE SYSTEM PROTOCOLS ....   ] ⚡"
    };
    const char netSpinner[] = { '/', '-', '\\', '|' };

    int frameIndex = 0;

    ULONG64 prevInBytes = 0, prevOutBytes = 0;
    ULONG64 currInBytes = 0, currOutBytes = 0;
    ULONG64 totalDownloadBytes = 0;
    ULONG64 totalUploadBytes = 0;

    getNetworkBytes(prevInBytes, prevOutBytes);
    getCPUUsage();
    system("cls");

    auto lastTime = std::chrono::high_resolution_clock::now();

    while (true) {
        setCursor(0, 0);

        std::cout << BOLD << MAGENTA << "========================================================\n";
        std::cout << "     🛰️  CYBERPUNK HARDWARE & NETWORK MONITOR v2.6     \n";
        std::cout << "========================================================\n\n" << RESET;

        std::cout << BOLD << YELLOW << radarFrames[frameIndex % radarFrames.size()] << RESET << "\n\n";

        double cpuUsage = getCPUUsage();
        std::cout << BOLD << " 🧠 CPU ACTIVITY:\n ";
        drawProgressBar(cpuUsage, 30);
        std::cout << "\n\n";

        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        double totalRamGB = static_cast<double>(memInfo.ullTotalPhys) / (1024 * 1024 * 1024);
        double avaliableRamGB = static_cast<double>(memInfo.ullAvailPhys) / (1024 * 1024 * 1024);
        double usedRamGB = totalRamGB - avaliableRamGB;
        std::cout << BOLD << " 🎛️  RAM LOAD:\n ";
        drawProgressBar(static_cast<double>(memInfo.dwMemoryLoad), 30);
        std::cout << " \n " << BLUE << "↳ Used: " << std::fixed << std::setprecision(2) << usedRamGB << " GB / " << totalRamGB << " GB" << RESET << "\n\n";

        getNetworkBytes(currInBytes, currOutBytes);
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = now - lastTime;
        double seconds = elapsed.count();

        double downloadSpeed = 0;
        double uploadSpeed = 0;

        if (seconds > 0 && prevInBytes > 0) {
            if (currInBytes >= prevInBytes) {
                downloadSpeed = static_cast<double>(currInBytes - prevInBytes) / seconds;
                totalDownloadBytes += (currInBytes - prevInBytes);
            }
            if (currOutBytes >= prevOutBytes) {
                uploadSpeed = static_cast<double>(currOutBytes - prevOutBytes) / seconds;
                totalUploadBytes += (currOutBytes - prevOutBytes);
            }
        }

        prevInBytes = currInBytes;
        prevOutBytes = currOutBytes;
        lastTime = now;

        char currentSpinner = netSpinner[frameIndex % 4];
        frameIndex++;

        std::cout << BOLD << CYAN << " 🌐 NETWORK TRAFFIC MONITOR [" << currentSpinner << "]:\n" << RESET;
        std::cout << "  📥 Download Speed : " << BOLD << GREEN << std::left << std::setw(15) << (formatBytes(downloadSpeed) + "/s")
            << RESET << " | Total Downloaded : " << BOLD << GREEN << formatBytes(static_cast<double>(totalDownloadBytes)) << RESET << "\n";

        std::cout << "  📤 Upload Speed   : " << BOLD << YELLOW << std::left << std::setw(15) << (formatBytes(uploadSpeed) + "/s")
            << RESET << " | Total Uploaded   : " << BOLD << YELLOW << formatBytes(static_cast<double>(totalUploadBytes)) << RESET << "\n";

        std::cout << "  📊 Combined Session Traffic: " << BOLD << MAGENTA << formatBytes(static_cast<double>(totalDownloadBytes + totalUploadBytes)) << RESET << "\n\n";

        std::cout << BOLD << RED << "====================== PROTOCOLS =======================\n" << RESET;
        std::cout << GREEN << " [+] Firewall: ACTIVE " << RESET << "       | " << CYAN << " [+] Core Temp: OPTIMAL\n" << RESET;
        std::cout << GREEN << " [+] Packet Integrity: SECURE" << RESET << " | " << CYAN << " [+] Active Threads: " << GetCurrentThreadId() << "\n" << RESET;
        std::cout << BOLD << RED << "========================================================\n" << RESET;

        std::cout << "\n Press " << BOLD << YELLOW << "Ctrl + C" << RESET << " to terminate link...";

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}