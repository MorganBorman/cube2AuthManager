enum {WIN_LINES = 0, OSX_LINES, NIX_LINES};

typedef struct {
    char *filename;
    int line_endings;
    int line_count;
    char **lines;
} FileBuffer;

void readfb(FileBuffer *fb, const char *filename);
void writefb(FileBuffer *fb);
void freefb(FileBuffer *fb);
