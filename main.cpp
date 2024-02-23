#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <cmath>
#include "cbmp.h"

std::vector<std::vector<std::vector<size_t>>> pixels; // each subvector is a pixels row vector that in turn is also a vector storing 3 values (R, G, B)
size_t width, height; // width and height of the image

const int PROCESSES_COUNT = 10;
size_t rCount = 0, gCount = 0, bCount = 0;

int threadIds[PROCESSES_COUNT];
pthread_mutex_t mutex;

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


void* ProcessFunction(ProcessFunctionArgs* structArgs) {
    std::cout << "Thread ID: " << structArgs->threadId << std::endl;

    for (auto x = structArgs->widthStart; x < structArgs->widthEnd; x++)
    {
        for (auto y = structArgs->heightStart; y < structArgs->heightEnd; y++)
        {
            auto r = pixels[x][y][0];
            auto g = pixels[x][y][1];
            auto b = pixels[x][y][2];

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
    return 0;
}

int main(int argc, char** argv)
{
    readBMP();

    auto colStep = std::floor(width / PROCESSES_COUNT);
    auto remainder = width % PROCESSES_COUNT;

    for (int i = 0; i < PROCESSES_COUNT; i++) {
        pid_t pid = fork();

        auto start = i * colStep;
        auto end = (i + 1) * colStep;

        if (i == PROCESSES_COUNT - 1) {
            end += remainder;
        }

        int threadId = i + 1;

        ProcessFunctionArgs* processFunctionArgs = new ProcessFunctionArgs();

        processFunctionArgs->threadId = threadId;
        processFunctionArgs->widthStart = start;
        processFunctionArgs->widthEnd = end;
        processFunctionArgs->heightStart = 0;
        processFunctionArgs->heightEnd = height;

        ProcessFunction(processFunctionArgs);

        if (pid == -1) {
            std::cerr << "Failed to create child process " << i << std::endl;
            return 1; // Handle the error accordingly
        } else if (pid == 0) {
            // Child process logic
            std::cout << "Hello from child process " << getpid() << std::endl;
            return 0; // Terminate the child process
        }
    }

    for (int i = 0; i < PROCESSES_COUNT; i++) {
        wait(nullptr);
    }

    std::cout << "R = " << rCount << std::endl;
    std::cout << "G = " << gCount << std::endl;
    std::cout << "B = " << bCount << std::endl;

    return 0;
}
