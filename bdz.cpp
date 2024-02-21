#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <Windows.h>
#include "cbmp.h"

std::vector<std::vector<std::vector<size_t>>> pixels; // each subvector is pixels row that holds array of 3 values (R, G, B)
size_t width, height; // width and height of the image

const int THREADS_COUNT = 10;
size_t rCount = 0, gCount = 0, bCount = 0;

HANDLE hThreads[THREADS_COUNT];
DWORD threadIds[THREADS_COUNT];

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

struct ThreadFunctionArgs {
    int threadId;
    size_t widthStart;
    size_t widthEnd;
    size_t heightStart;
    size_t heightEnd;
};


DWORD WINAPI ThreadFunction(void* arg) {
    ThreadFunctionArgs* structArgs = (struct ThreadFunctionArgs*)arg;

    //std::cout << "Thread ID: " << structArgs->threadId << std::endl;

    for (auto x = structArgs->widthStart; x < structArgs->widthEnd; x++)
    {
        for (auto y = structArgs->heightStart; y < structArgs->heightEnd; y++)
        {
            auto r = pixels[x][y][0];
            auto g = pixels[x][y][1];
            auto b = pixels[x][y][2];

            if (int(r) >= int(g) && int(r) >= int(b)) {
                InterlockedIncrement(&rCount);
            }
            else if (int(g) >= int(r) && int(g) >= int(b)) {
                InterlockedIncrement(&gCount);
            }
            else if (int(b) >= int(r) && int(b) >= int(g)) {
                InterlockedIncrement(&bCount);
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

    auto colStep = std::floor(width / THREADS_COUNT);
    auto remainder = std::remainder(width, THREADS_COUNT);

    for (auto i = 0; i < THREADS_COUNT; ++i) {
        auto start = i * colStep;
        auto end = (i + 1) * colStep;

        if (i == THREADS_COUNT - 1) {
            end += remainder;
        }

        int threadId = i + 1;
        threadIds[i] = threadId;

        ThreadFunctionArgs* threadFunctionArgs = new ThreadFunctionArgs();

        threadFunctionArgs->threadId = threadId;
        threadFunctionArgs->widthStart = start;
        threadFunctionArgs->widthEnd = end;
        threadFunctionArgs->heightStart = 0;
        threadFunctionArgs->heightEnd = height;

        //std::cout << "Inside loop: " << threadFunctionArgs.threadId << std::endl;


        hThreads[i] = CreateThread(NULL, 0, ThreadFunction, threadFunctionArgs, 0, &threadIds[i]);

        if (hThreads[i] == NULL)
        {
            std::cerr << "Failed to create thread " << i << std::endl;
            return 1; // Handle the error accordingly
        }
    }

    // Wait for all threads to finish
    WaitForMultipleObjects(THREADS_COUNT, hThreads, TRUE, INFINITE);

    for (int i = 0; i < THREADS_COUNT; ++i)
    {
        CloseHandle(hThreads[i]);
    }

    std::cout << "R = " << rCount << std::endl;
    std::cout << "G = " << gCount << std::endl;
    std::cout << "B = " << bCount << std::endl;

    return 0;
}