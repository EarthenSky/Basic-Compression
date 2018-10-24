#ifndef UTIL
#define UTIL

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

// Include utility functions here:
// -Read png.
// -Write to file.
// Write to png.

// This function sends the command args to a certain compression type. (function)
//typedef int (*fp)(char*, char*) t_defaultFunction;
void sendArgs( int (*fp)(char*, char*, char), int argc, char* argv[] ) {
    if (argc < 4) {
        printf("Error: Not enough options.  Use --help to see needed options.\n");  // Give error.
        return;  // Exit before doing anything.
    }

    char* operationIn = argv[2];
    char* inputFilepath = argv[3];
    char* outputFilepath = argv[4];

    // Simplify the operation switch.
    char operation;
    if(strcmp(operationIn, "-c")==0 || strcmp(operationIn, "--compress")==0) {
        operation = 'c';
    } else if(strcmp(operationIn, "-d")==0 || strcmp(operationIn, "--decompress")==0) {
        operation = 'd';
    } else {
        printf("Error: Inproper option \"%s\" see --help for information on options / ordering.\n", operationIn);  // Give error.
        return;  // Exit before doing anything.
    }

    // Check if file exists at location.
    if ( access(inputFilepath, F_OK) == -1 ) {
        printf("Invalid filepath: \"%s\"\nMake sure to include the filename in the path.\n", inputFilepath);  // Give error.
        return;  // Exit before doing anything.
    }

    // Make sure user gave an output filepath.
    if ( outputFilepath == NULL ) {
        printf("Error: No output filepath given. Remember to give an input, then output filepath.\n");  // Give error.
        return;  // Exit before doing anything.
    }

    int returnValue = fp(inputFilepath, outputFilepath, operation);  // Call the function and send variables.
    //printf("Function returned with a %i\n", returnValue);
}

// This function writes a string to the location.  Doesn't print any \0s.
void writeFile(char* writeLocation, char* &fileString) {
    //TODO: check if file is overwriting another file.

    FILE* fptr = fopen(writeLocation, "w");

    if (fptr == NULL) {
        printf("Error: failed to write file.\n");
        return;
    }

    fputs (fileString, fptr);  // Case: stop at first \0

    fclose(fptr);
    printf("File has been written to %s\n", writeLocation);
    free (fileString);  // Must be freed.
}

// This function writes a string to the location.
void writeFileWithLength(char* writeLocation, char* &fileString, long long stringSize) {
    //TODO: check if file is overwriting another file.

    FILE* fptr = fopen(writeLocation, "wb");

    if (fptr == NULL) {
        printf("Error: failed to write file.\n");
        return;
    }

    for(int i=0; i<stringSize; i++) {
        unsigned char tmpCh[1] = { (unsigned char) fileString[i] };
        fwrite ((int*) tmpCh, 1, 1, fptr);
    }

    fclose(fptr);
    printf("File has been written to %s\n", writeLocation);
    free (fileString);  // Must be freed.
}


/*8�%@
�*/

/*8ÿ%@
ÿ9	*/
#endif //!UTIL
