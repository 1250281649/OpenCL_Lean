#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <fstream>
#include <CL/cl.h>

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


int main()
{
	printf("test image rotate");
	
}