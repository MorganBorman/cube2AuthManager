#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filebuffer.h"

void readfb(FileBuffer *fb, const char *filename)
{
    long int line_count = 0;
    long int line_length = 0;

    int filename_length = strlen(filename);

    fb->filename = (char*)malloc(sizeof(char)*(filename_length+1));
    strcpy(fb->filename, filename);

    FILE *fp = fopen(filename, "rb");
    
    int prev_char = -1;
    int curr_char = -1;
    int next_char = -1;
    
    next_char = fgetc(fp);
    
    while (1) {
        prev_char = curr_char;
        curr_char = next_char;
        if(curr_char != EOF) next_char = fgetc(fp);
        
        //If we found a line terminator
        if((curr_char == '\n' && prev_char != '\r') || (curr_char == '\r' && next_char != '\n') || (curr_char == '\r' && next_char == '\n') || (curr_char == EOF))
        {
            line_count++;
        }
		
		if(curr_char == EOF) break;
    }
	
	printf("Found %ld lines in file %s.\n", line_count, fb->filename);
    
    fseek(fp, 0, SEEK_SET);
    
    fb->lines = (char**)malloc(sizeof(char *)*(line_count));
    fb->line_count = line_count;
    
    line_count = 0;
    
    long int start_pos;
    long int end_pos;
    
    start_pos = ftell(fp);
    next_char = fgetc(fp);
    
    while (1) {
        prev_char = curr_char;
        curr_char = next_char;
        if(curr_char != EOF) next_char = fgetc(fp);
        
        //If we found a line terminator
        if((curr_char == '\n' && prev_char != '\r') || (curr_char == '\r' && next_char != '\n') || (curr_char == '\r' && next_char == '\n') || (curr_char == EOF))
        {
			if(curr_char == EOF) line_length--;
		
            //Declare space for the line
            fb->lines[line_count] = (char *)malloc(sizeof(char)*(line_length+1));
            
            //Store the position of the second terminator
            end_pos = ftell(fp);
            
            #define WINDINGS (curr_char == '\r' && next_char == '\n')
            
            //return to the front of the line
            fseek(fp, start_pos, SEEK_SET);
            
			printf("Read line of %d characters from %s.\n", line_length, fb->filename);
			
            //Read and terminate the line
            fread(fb->lines[line_count], sizeof(char), line_length, fp);
            fb->lines[line_count][line_length] = '\0';
            
            if(WINDINGS)
                fseek(fp, end_pos+1, SEEK_SET);
            else
                fseek(fp, end_pos, SEEK_SET);
            
            line_length = 0;
            line_count++;
            
            if(curr_char == '\n' && prev_char != '\r') fb->line_endings = NIX_LINES;
            else if(curr_char == '\r' && next_char != '\n') fb->line_endings = OSX_LINES;
            else if(curr_char == '\r' && next_char == '\n') fb->line_endings = WIN_LINES;
            
        }
        else line_length++;
		
		if(curr_char == EOF) break;
        
        if(!line_length) start_pos = ftell(fp) - 1;
    }
    
    fclose(fp);
}

void writefb(FileBuffer *fb)
{
    FILE *fp = fopen(fb->filename, "w");

    int lx = 0;
    for(lx = 0; lx < fb->line_count; lx++)
    {
        if(!fb->lines[lx]) continue;
        
		printf("Writing line to file %s: '%s'\n", fb->filename, fb->lines[lx]);
		
        fprintf(fp, "%s\n", fb->lines[lx]);
    }
    
    fclose(fp);
}

void freefb(FileBuffer *fb)
{
    int lx = 0;
    for(lx = 0; lx < fb->line_count; lx++)
    {
        if(!fb->lines[lx]) continue;
    
        free(fb->lines[lx]);
    }
    free(fb->lines);
    free(fb->filename);
}
