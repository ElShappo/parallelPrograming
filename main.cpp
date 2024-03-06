#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>
#include <cmath>
#include <cstring>
#include "mpi.h"
#include "cbmp.h"

const int PROCESSES_COUNT = 10;
size_t width, height; // width and height of the image

std::vector<int> readBMP(int processId) {
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
    return {rCount, gCount, bCount};
}

int main(int argc, char** argv) {
  MPI_Init(NULL, NULL);
  int world_rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  int world_size;
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  std::vector<int> localRgbCount = readBMP(world_rank);
  int* rgbCountList = nullptr;

  if (world_rank == 0) {
    rgbCountList = new int[PROCESSES_COUNT * 3];
  }

  MPI_Gather(localRgbCount.data(), localRgbCount.size(), MPI_INT, rgbCountList, localRgbCount.size(), MPI_INT, 0,
           MPI_COMM_WORLD);

    if (world_rank == 0) {
        int rCount = 0;
        int gCount = 0;
        int bCount = 0;

        // std::cout << "test" << std::endl;

        for (int i = 0; i < PROCESSES_COUNT * 3; i += 3) {
            rCount += rgbCountList[i];
            gCount += rgbCountList[i + 1];
            bCount += rgbCountList[i + 2];
        }

        std::cout << "R = " << rCount << std::endl;
        std::cout << "G = " << gCount << std::endl;
        std::cout << "B = " << bCount << std::endl;
    }

  MPI_Finalize();
}
