#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <vector>
#include <pthread.h>
#include <cmath>
#include "cbmp.h"

std::vector<std::vector<std::vector<size_t>>> pixels; // each subvector is a pixels row vector that in turn is also a vector storing 3 values (R, G, B)
size_t width, height; // width and height of the image

const int THREADS_COUNT = 10;
size_t rCount = 0, gCount = 0, bCount = 0;

pthread_t threads[THREADS_COUNT];
int threadIds[THREADS_COUNT];
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

struct ThreadFunctionArgs {
    int threadId;
    size_t widthStart;
    size_t widthEnd;
    size_t heightStart;
    size_t heightEnd;
};


void* ThreadFunction(void* arg) {
    ThreadFunctionArgs* structArgs = (struct ThreadFunctionArgs*)arg;

    std::cout << "Thread ID: " << structArgs->threadId << std::endl;

    for (auto x = structArgs->widthStart; x < structArgs->widthEnd; x++)
    {
        for (auto y = structArgs->heightStart; y < structArgs->heightEnd; y++)
        {
            pthread_mutex_lock(&mutex);

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

            pthread_mutex_unlock(&mutex);
        }
    }
    return 0;
}

int main(int argc, char** argv)
{
    readBMP();

    pthread_mutex_init(&mutex, NULL);

    auto colStep = std::floor(width / THREADS_COUNT);
    auto remainder = width % THREADS_COUNT;

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

        int threadCreateResult = pthread_create(&threads[i], NULL, &ThreadFunction, threadFunctionArgs);

        if (threadCreateResult != 0) {
            std::cerr << "Failed to create thread " << i << std::endl;
            return 1;
        }
    }

    for (int i = 0; i < THREADS_COUNT; ++i) {
        int threadJoinResult = pthread_join(threads[i], NULL);
        if (threadJoinResult != 0) {
            std::cerr << "Failed to join thread " << i << std::endl;
            return 1; // Handle the error accordingly
        }
    }

    pthread_mutex_destroy(&mutex);

    std::cout << "R = " << rCount << std::endl;
    std::cout << "G = " << gCount << std::endl;
    std::cout << "B = " << bCount << std::endl;

    return 0;
}
