#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include <cstring>
#include <signal.h>
#include "cbmp.h"

std::vector<std::vector<std::vector<size_t>>> pixels; // each subvector is a pixels row vector that in turn is also a vector storing 3 values (R, G, B)
size_t width, height; // width and height of the image

const int PROCESSES_COUNT = 10;

void readBMP() {
    std::string filename = "red.bmp";
    BMP* bmp = bopen((char*)filename.c_str());
    width = get_width(bmp), height = get_height(bmp);

    unsigned char r, g, b;

    for (int x = width - 1; x >= 0; --x)
    {
        std::vector<std::vector<size_t>> pixelsRow;

        for (int y = height - 1; y >= 0; --y)
        {
            get_pixel_rgb(bmp, x, y, &r, &g, &b);
            std::vector<size_t> pixel = { r, g, b };
            pixelsRow.push_back(pixel);

            //std::cout << "R: " << int(r) << std::endl;
            //std::cout << "G: " << int(g) << std::endl;
            //std::cout << "B: " << int(b) << std::endl << std::endl;
        }
        pixels.push_back(pixelsRow);
    }
    bclose(bmp);
}

struct ProcessFunctionArgs {
    int threadId;
    size_t widthStart;
    size_t widthEnd;
    size_t heightStart;
    size_t heightEnd;
};


std::vector<int> ProcessFunction(ProcessFunctionArgs* structArgs) {
    std::vector<int> rgbCount = {0, 0, 0}; // rCount, gCount, bCount
//    std::cout << "Process ID: " << structArgs->threadId << std::endl;

    for (auto x = structArgs->widthStart; x < structArgs->widthEnd; x++)
    {
        for (auto y = structArgs->heightStart; y < structArgs->heightEnd; y++)
        {
            auto r = pixels[x][y][0];
            auto g = pixels[x][y][1];
            auto b = pixels[x][y][2];

            if (int(r) >= int(g) && int(r) >= int(b)) {
                ++rgbCount[0];
            }
            else if (int(g) >= int(r) && int(g) >= int(b)) {
                ++rgbCount[1];
            }
            else if (int(b) >= int(r) && int(b) >= int(g)) {
                ++rgbCount[2];
            }
            else {
                // do nothing
            }
        }
    }
    return rgbCount;
}

int main(int argc, char** argv)
{
    readBMP();

    auto colStep = std::floor(width / PROCESSES_COUNT);
    auto remainder = width % PROCESSES_COUNT;

    int fd[2]; // file descriptors

    if (pipe(fd) == -1) {
        std::cout << "Error occurred" << std::endl;
    }

    int zeros[3] = { };
    write(fd[1], zeros, sizeof(int) * 3);

    for (int i = 0; i < PROCESSES_COUNT; i++) {
        pid_t pid = fork();

        auto start = i * colStep;
        auto end = (i + 1) * colStep;

        if (i == PROCESSES_COUNT - 1) {
            end += remainder;
        }

        int threadId = i + 1;

        if (pid == -1) {
            std::cerr << "Failed to create child process " << i << std::endl;
            return 1;
        } else if (pid == 0) {
            // std::cout << "Hello from child process " << getpid() << std::endl;

            ProcessFunctionArgs* processFunctionArgs = new ProcessFunctionArgs();

            processFunctionArgs->threadId = threadId;
            processFunctionArgs->widthStart = start;
            processFunctionArgs->widthEnd = end;
            processFunctionArgs->heightStart = 0;
            processFunctionArgs->heightEnd = height;

            auto rgbCount = ProcessFunction(processFunctionArgs);

            int receivedRGBCount[3];

            if (read(fd[0], receivedRGBCount, sizeof(int) * 3) < 0) {
                return 1;
            }
            close(fd[0]);
            for (int i = 0; i < 3; ++i) {
                receivedRGBCount[i] += rgbCount[i];
            }

            write(fd[1], receivedRGBCount, sizeof(int) * 3);
            close(fd[1]); // Close the write end of the pipe in the child process
            kill(getpid(), SIGKILL);
        }
    }

    for (int i = 0; i < PROCESSES_COUNT; i++) {
        wait(nullptr);
    }

    close(fd[1]);

    int buffer;
    std::vector<int> receivedNumbers;
    const int bufferSize = sizeof(int);

    while (read(fd[0], &buffer, bufferSize) > 0) {
        receivedNumbers.push_back(buffer);
    }

    close(fd[0]);

    std::cout << "R = " << receivedNumbers[0] << std::endl;
    std::cout << "G = " << receivedNumbers[1] << std::endl;
    std::cout << "B = " << receivedNumbers[2] << std::endl;

    return 0;
}
