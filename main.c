// g++ main.c util.h simple.c simple.h dense.c dense.h stb_image.h stb_image_write.h -o comp.exe
// comp simple -c C:/images/forest.png C:/images/forest.simple
// comp dense -d C:/images/forest.dense C:/images/forest6.png
// g++ main.c util.h simple.c simple.h dense.c dense.h stb_image.h stb_image_write.h -o comp.exe -Wall
// -Wall is a life saver!

#include <stdio.h>
#include <string.h>
#include "util.h"  // Utility functions.

//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"  // For reading images
//#endif

//#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
//#define STB_IMAGE_WRITE_IMPLEMENTATION
//#include "stb_image_write.h"
//#endif

#include "simple.h"  // include the simple compression type.
#include "dense.h"

void printHelp( void );

int main(int argc, char *argv[]) {
    //printf("All Arguments:\n");
    //for (int i=0; i<argc; i++) {
    //    printf("\t%i: %s\n", i, argv[i]);
    //}

    // Check if user needs the help page.
    if (strcmp(argv[1], "--help")==0 || strcmp(argv[1], "-h")==0 || strcmp(argv[1], "--info")==0) {
        printHelp();
        return 0;  // Exit program.
    } else if(strcmp(argv[1], "simple")==0) {  // Case: compress the simple file type.
        sendArgs(simple, argc, argv);  // Send the arguments (formatted) to the simple algorithm.
        return 0;  // Exit program.
    } else if(strcmp(argv[1], "dense")==0) {  // Case: compress the simple file type.
        sendArgs(dense, argc, argv);  // Send the arguments (formatted) to the simple algorithm.
        return 0;  // Exit program.
    }

    printf("Unknown command \"%s\", use --help to open the help page.\n", argv[1]);
    return 0;
}

// This function gives the user help on using the program.
void printHelp( void ) {
    printf("Usage:\n");
    printf("  main.exe simple (-c|-d) <input filepath> <output filepath>\n");
    printf("  main.exe --help | -h | --info \n");
    printf("Options:\n");
    printf("  -c --compress \tThis option compresses a png file.\n");
    printf("  -d --decompress \tThis option decompresses a compressed file.\n");
    printf("  --help | -h | --info \tShow this screen.\n");
    printf("Directions:\n");
    printf("  To compress a .png image include the compression type, the filepath to the image, then append it with -c.\n");
    printf("  This generates a file with the exact same filename but with a new file extension.\n");
    printf("  Will not overwrite files with the exact same name.\n");
}
