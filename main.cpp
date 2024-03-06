#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <Windows.h>
#include <fstream>
#include "cbmp.h"

const int PROCESSES_COUNT = 10;

void readBMP(int processId) {
    std::string filename = "red.bmp";
    BMP* bmp = bopen((char*)filename.c_str());
    size_t width = get_width(bmp);
    size_t height = get_height(bmp);

    unsigned char r, g, b;
    std::vector<std::vector<int>> pixels;

    auto colStep = std::floor(width / PROCESSES_COUNT);
    auto remainder = width % PROCESSES_COUNT;

    int x = width - 1 - (colStep - 1) * processId;
    int end_ = x - colStep + 1;

    if (processId == PROCESSES_COUNT - 1) {
        colStep += remainder;
        x = colStep - 1;
        end_ = 0;
    }

    /*
    std::cout << "width: " << width << std::endl;
    std::cout << "height: " << height << std::endl;
    std::cout << "colStep: " << colStep << std::endl;


    std::cout << "x: " << x << std::endl;
    std::cout << "end: " << end_ << std::endl;
    */

    int rCount = 0, gCount = 0, bCount = 0;
    for (; x >= end_; --x)
    {
        for (int y = height - 1; y >= 0; --y)
        {
            get_pixel_rgb(bmp, x, y, &r, &g, &b);
            std::vector<int> pixel = { r, g, b };
            pixels.push_back(pixel);

            if (int(r) >= int(g) && int(r) >= int(b)) {
                ++rCount;
            }
            else if (int(g) >= int(r) && int(g) >= int(b)) {
                ++gCount;
            }
            else if (int(b) >= int(r) && int(b) >= int(g)) {
                ++bCount;
            }
            else {
                // do nothing
            }
        }
    }

    bclose(bmp);

    std::string outputFilePath = "files/ans_part" + std::to_string(processId) + ".txt";

    /*
    std::cout << "R: " << rCount << std::endl;
    std::cout << "G: " << gCount << std::endl;
    std::cout << "B: " << bCount << std::endl;
    */

    std::ofstream outputFile(outputFilePath);

    if (outputFile.is_open()) {
        outputFile << rCount << std::endl;
        outputFile << gCount << std::endl;;
        outputFile << bCount << std::endl;;
        outputFile.close();
    } else {
        std::cout << "Error opening the file." << std::endl;
    }
}

int main(int argc, TCHAR *argv[])
{
    if (argc == 1) {
        // std::cout << argv[0] << std::endl;
        HANDLE processHandles[PROCESSES_COUNT];

        for (int i = 0; i < PROCESSES_COUNT; i++) {
            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            ZeroMemory( &pi, sizeof(pi) );

            std::string path(argv[0]);
            std::string commandLineArguments = path + " " + std::to_string(i);
            // std::cout << "cl args: " << commandLineArguments.c_str() << std::endl;

            if( !CreateProcess(NULL,   // No module name (use command line)
                const_cast<LPSTR>(commandLineArguments.c_str()),        // Command line
                NULL,           // Process handle not inheritable
                NULL,           // Thread handle not inheritable
                FALSE,          // Set handle inheritance to FALSE
                0,              // No creation flags
                NULL,           // Use parent's environment block
                NULL,           // Use parent's starting directory
                &si,            // Pointer to STARTUPINFO structure
                &pi )           // Pointer to PROCESS_INFORMATION structure
            )
            {
                std::cout << "CreateProcess failed with code " << GetLastError() << std::endl;
                return 1;
            }
            processHandles[i] = pi.hProcess;
        }

        // Wait for all processes to exit
        WaitForMultipleObjects(PROCESSES_COUNT, processHandles, TRUE, INFINITE);

        // Close process handles
        for (int i = 0; i < PROCESSES_COUNT; ++i) {
            CloseHandle( processHandles[i] );
        }

        int rCount = 0, gCount = 0, bCount = 0;

        for (int i = 0; i < PROCESSES_COUNT; ++i) {
            std::string filePath = "files/ans_part" + std::to_string(i) + ".txt";
            std::ifstream inputFile(filePath);   // Open the file for reading

            if (inputFile.is_open()) {
                std::string line;

                std::getline(inputFile, line);
                rCount += std::stoi(line);

                std::getline(inputFile, line);
                gCount += std::stoi(line);

                std::getline(inputFile, line);
                bCount += std::stoi(line);

                inputFile.close();   // Close the file
                // std::cout << "File read complete." << std::endl;
            } else {
                std::cout << "Error opening the file." << std::endl;
            }
        }
        std::cout << "R: " << rCount << std::endl;
        std::cout << "G: " << gCount << std::endl;
        std::cout << "B: " << bCount << std::endl;

    } else {
        int processId = std::stoi(std::string(argv[1]));
        // std::cout << processId << std::endl;
        readBMP(processId);
        return 0;
    }

}
