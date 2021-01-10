#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <Windows.h>
#include <CL/cl.h>

//#define BUFSIZE     (pow(2, 15))

int convertToString(const char* fileName, std::string& s)
{
    size_t size;
    char* str;

    std::fstream f(fileName, (std::fstream::in | std::fstream::binary));

    if (f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size + 1];
        if (!str)
        {
            f.close();
            return -1;
        }
        f.read(str, fileSize);
        f.close();
        str[size] = '\0';

        s = str;
        delete[] str;
        return 0;
    }
    printf("Failed to open file %s\n", fileName);
    return -1;
}

//void scalar_add(int n, const float* a, const float* b, float* result)
//{
//    int i = 0;
//    for (i = 0; i < n; i++)
//    {
//        result[i] = a[i] + b[i];
//    }
//}