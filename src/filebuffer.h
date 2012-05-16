#include <string>
#include <vector>

typedef struct {
    char *filename;
    std::vector<std::string> lines;
} FileBuffer;

void readfb(FileBuffer *fb, const char *filename);
void writefb(FileBuffer *fb);
void freefb(FileBuffer *fb);
