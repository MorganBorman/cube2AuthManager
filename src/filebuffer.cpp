#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <iostream>
#include <fstream>

#include "filebuffer.h"

void readfb(FileBuffer *fb, const char *filename)
{
    int filename_length = strlen(filename);

    fb->filename = (char*)malloc(sizeof(char)*(filename_length+1));
    strcpy(fb->filename, filename);

    std::ifstream file(fb->filename);
    std::string temp;

    while( getline(file, temp))
        fb->lines.push_back(temp);
}

void writefb(FileBuffer *fb)
{
    FILE *fp = fopen(fb->filename, "w");
    
    for(std::vector<std::string>::iterator it = fb->lines.begin(); it != fb->lines.end(); ++it)
    {
        fprintf(fp, "%s\n", (*it).c_str());
    }

    fclose(fp);
}

void freefb(FileBuffer *fb)
{
    fb->lines.clear();
    free(fb->filename);
}
