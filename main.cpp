#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <cmath>
#include "cbmp.h"

std::mutex mtx;

std::vector<std::vector<std::vector<size_t>>> pixels; // each subvector is pixels row that holds array of 3 values (R, G, B)
size_t width, height; // width and height of the image

size_t THREADS_COUNT = 10;
size_t rCount = 0, gCount = 0, bCount = 0;

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
            // Gets pixel rgb values at point (x, y)
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


// this function is used as a thread function
void countRGB(
    size_t widthStart, size_t widthEnd,
    size_t heightStart, size_t heightEnd)
{
    for (auto x = widthStart; x < widthEnd; x++)
    {
        for (auto y = heightStart; y < heightEnd; y++)
        {

            auto r = pixels[x][y][0];
            auto g = pixels[x][y][1];
            auto b = pixels[x][y][2];

            mtx.lock();

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
            mtx.unlock();
        }
    }
}

int main(int argc, char** argv)
{
    readBMP();

    auto colStep = std::floor(width / THREADS_COUNT);
    std::vector<std::thread> threads;

    auto remainder = width % THREADS_COUNT;

    for (auto i = 0; i < THREADS_COUNT; ++i) {
        auto start = i * colStep;
        auto end = (i + 1) * colStep;

        if (i == THREADS_COUNT - 1) {
            end += remainder;
        }

        threads.push_back(std::thread(countRGB, start, end, 0, height));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "R = " << rCount << std::endl;
    std::cout << "G = " << gCount << std::endl;
    std::cout << "B = " << bCount << std::endl;

    return 0;
}
