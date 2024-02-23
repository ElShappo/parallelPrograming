#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <thread>
#include <mutex>
#include "cbmp.h"

std::mutex mtx;

void countRGB(BMP *bmp, 
    size_t &rCount, size_t &gCount, size_t &bCount,
    size_t rowStart, size_t rowEnd,
    size_t colStart, size_t colEnd) 
{
    unsigned char r, g, b;

    for (auto x = rowStart; x < rowEnd; x++)
    {
        for (auto y = colStart; y < colEnd; y++)
        {
            // Gets pixel rgb values at point (x, y)
            get_pixel_rgb(bmp, x, y, &r, &g, &b);

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
    std::string filename = "blue.bmp";
    BMP* bmp = bopen((char*)filename.c_str());

    unsigned int width = get_width(bmp), height = get_height(bmp);
    size_t rCount = 0, gCount = 0, bCount = 0;

    auto widthMid = std::floor(width / 2);
    auto heightMid = std::floor(height / 2);

    std::thread th(countRGB, bmp, std::ref(rCount), std::ref(gCount), std::ref(bCount), 0, widthMid, 0, height);
    countRGB(bmp, rCount, gCount, bCount, widthMid, width, 0, height);

    th.join();

    // Free memory
    bclose(bmp);

    std::cout << "R = " << rCount << std::endl;
    std::cout << "G = " << gCount << std::endl;
    std::cout << "B = " << bCount << std::endl;

    return 0;
}
