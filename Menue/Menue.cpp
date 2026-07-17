#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <sstream>
#include <algorithm>
#include <conio.h> 

namespace fs = std::filesystem;

const std::string RESET = "\033[0m";
const std::string BOLD = "\033[1m";
const std::string BLUE = "\033[34m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string CYAN = "\033[36m";
const std::string RED = "\033[31m";
const std::string MAGENTA = "\033[35m";
const std::string BG_BLUE = "\033[44m"; 

std::string formatSize(uintmax_t bytes);
std::string toLower(std::string str);
void listDirectory();
void searchFilesHTML();
void copyFileWithProgress();

void clearScreen() {
#ifdef _WIN32
    std::system("cls");
#else
    std::system("clear");
#endif
}


int showMenu(const std::vector<std::string>& options) {
    int highlight = 0;
    while (true) {
        clearScreen();
        std::cout << BOLD << MAGENTA << "====================================================\n" << RESET;
        std::cout << BOLD << " 🌌 LUXURY FILE MANAGEMENT SYSTEM - MAIN MENU\n";
        std::cout << BOLD << " Use " << YELLOW << "↑ ↓" << RESET << " arrows to navigate and " << GREEN << "Enter" << RESET << " to select.\n";
        std::cout << BOLD << MAGENTA << "====================================================\n\n" << RESET;

        for (size_t i = 0; i < options.size(); ++i) {
            if (highlight == static_cast<int>(i)) {
                
                std::cout << BOLD << BG_BLUE << CYAN << "  ▶  " << std::left << std::setw(40) << options[i] << RESET << "\n";
            }
            else {
                std::cout << "     " << std::left << std::setw(40) << options[i] << "\n";
            }
        }
        std::cout << BOLD << MAGENTA << "\n====================================================\n" << RESET;

       
        int ch = _getch();
        if (ch == 0 || ch == 224) { 
            switch (_getch()) {
            case 72: 
                highlight = (highlight - 1 + options.size()) % options.size();
                break;
            case 80: 
                highlight = (highlight + 1) % options.size();
                break;
            }
        }
        else if (ch == 13) { 
            return highlight;
        }
    }
}

int main() {
    std::vector<std::string> menuOptions = {
        "1. List Directory Contents & Save TXT",
        "2. Advanced File Search & Generate HTML",
        "3. Copy File with Live Progress Bar",
        "4. Exit Program"
    };

    while (true) {
        int choice = showMenu(menuOptions);
        clearScreen();

        switch (choice) {
        case 0:
            listDirectory();
            break;
        case 1:
            searchFilesHTML();
            break;
        case 2:
            copyFileWithProgress();
            break;
        case 3:
            std::cout << BOLD << GREEN << "\n Thank you for using Luxury File System! Goodbye. ✨\n\n" << RESET;
            return 0;
        }

        std::cout << BOLD << YELLOW << "\nPress any key to return to Main Menu..." << RESET;
        _getch(); 
    }
    return 0;
}


std::string formatSize(uintmax_t bytes) {
    if (bytes == 0) return "0 B";
    const std::string suffix[] = { "B", "KB", "MB", "GB", "TB" };
    int i = 0;
    double double_bytes = static_cast<double>(bytes);
    while (double_bytes >= 1024 && i < 4) { double_bytes /= 1024; i++; }
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << double_bytes << " " << suffix[i];
    return ss.str();
}

std::string toLower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
    return str;
}

void listDirectory() {
    std::string path;
    std::cout << BOLD << BLUE << "[1. DIRECTORY LISTER]\n" << RESET << "Enter folder path: ";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); 
    std::getline(std::cin, path);

    int fileCount = 0, folderCount = 0;
    try {
        if (!fs::exists(path) || !fs::is_directory(path)) {
            std::cout << RED << "Error: Invalid Path.\n" << RESET; return;
        }
        std::ofstream outputFile("report.txt");
        outputFile << "DIRECTORY REPORT FOR: " << path << "\n\n";

        std::cout << "\n" << BOLD << YELLOW << std::left << std::setw(15) << "Type" << std::setw(40) << "Name" << std::setw(15) << "Size" << "\n-------------------------------------------------------------------\n" << RESET;
        for (const auto& entry : fs::directory_iterator(path)) {
            std::string filename = entry.path().filename().string();
            if (filename.length() > 37) filename = filename.substr(0, 34) + "...";

            if (entry.is_directory()) {
                folderCount++;
                outputFile << "[DIR] " << filename << "\n";
                std::cout << BLUE << std::left << std::setw(15) << "📁 [DIR]" << RESET << BOLD << std::setw(40) << filename << RESET << "-\n";
            }
            else if (entry.is_regular_file()) {
                fileCount++;
                std::string fSize = formatSize(fs::file_size(entry.path()));
                outputFile << "[FILE] " << std::setw(40) << filename << fSize << "\n";
                std::cout << GREEN << std::left << std::setw(15) << "📄 [FILE]" << RESET << std::setw(40) << filename << CYAN << fSize << RESET << "\n";
            }
        }
        outputFile.close();
        std::cout << BOLD << YELLOW << "-------------------------------------------------------------------\n" << RESET;
        std::cout << "✔ Saved to 'report.txt' | Total Files: " << fileCount << " | Folders: " << folderCount << "\n";
    }
    catch (...) { std::cout << RED << "An error occurred.\n" << RESET; }
}

void searchFilesHTML() {
    std::string path, keyword;
    std::cout << BOLD << GREEN << "[2. ADVANCED HTML SEARCH]\n" << RESET << "Enter folder path: ";
    std::getline(std::cin, path);
    std::cout << "Enter keyword/extension (Leave empty for ALL): ";
    std::getline(std::cin, keyword);
    std::string lowerKeyword = toLower(keyword);

    int fileCount = 0, folderCount = 0, matchCount = 0;
    try {
        if (!fs::exists(path) || !fs::is_directory(path)) { std::cout << RED << "Error: Invalid Path.\n" << RESET; return; }
        std::ofstream html("search_report.html");
        html << "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>Report</title><style>body{font-family:sans-serif;background:#1e1e24;color:#fff;padding:20px;}table{width:100%;border-collapse:collapse;margin-top:20px;}th,td{padding:12px;text-align:left;border-bottom:1px solid #3a3a4a;}th{background:#333344;color:#4fc3f7;}</style></head><body><h1>🔍 Search Results</h1><p>Path: " << path << "</p><table><tr><th>Type</th><th>Name</th><th>Size</th></tr>";

        for (const auto& entry : fs::directory_iterator(path)) {
            std::string filename = entry.path().filename().string();
            if (entry.is_directory()) folderCount++; else fileCount++;

            if (lowerKeyword.empty() || toLower(filename).find(lowerKeyword) != std::string::npos) {
                matchCount++;
                if (entry.is_directory()) {
                    html << "<tr><td>📁 DIR</td><td><b>" << filename << "</b></td><td>-</td></tr>";
                }
                else {
                    html << "<tr><td>📄 FILE</td><td>" << filename << "</td><td>" << formatSize(fs::file_size(entry.path())) << "</td></tr>";
                }
            }
        }
        html << "</table></body></html>"; html.close();
        std::cout << GREEN << "\n✔ HTML Report generated! Matches found: " << matchCount << RESET << "\n";
    }
    catch (...) { std::cout << RED << "An error occurred.\n" << RESET; }
}

void copyFileWithProgress() {
    std::string src, dest;
    std::cout << BOLD << CYAN << "[3. LUXURY COPIER]\n" << RESET << "Enter SOURCE file path: ";
    std::getline(std::cin, src);
    std::cout << "Enter DESTINATION file path: ";
    std::getline(std::cin, dest);
    try {
        if (!fs::exists(src) || fs::is_directory(src)) { std::cout << RED << "Source doesn't exist.\n" << RESET; return; }
        uintmax_t size = fs::file_size(src);
        std::ifstream sFile(src, std::ios::binary); std::ofstream dFile(dest, std::ios::binary);

        const size_t bufSize = 64 * 1024; char* buf = new char[bufSize];
        uintmax_t copied = 0;
        auto start = std::chrono::high_resolution_clock::now();

        std::cout << "\n";
        while (sFile.good() && dFile.good()) {
            sFile.read(buf, bufSize);
            std::streamsize read = sFile.gcount();
            if (read == 0) break;
            dFile.write(buf, read);
            copied += read;

            double pct = (static_cast<double>(copied) / size) * 100.0;
            std::cout << "\r" << BOLD << CYAN << "[Copying... " << std::fixed << std::setprecision(1) << pct << "%]" << RESET << std::flush;
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        delete[] buf; sFile.close(); dFile.close();
        std::cout << GREEN << "\n\n✨ File copied successfully!\n" << RESET;
    }
    catch (...) { std::cout << RED << "Copy failed.\n" << RESET; }
}