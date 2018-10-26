#include "dense.h"

/// DESCRIPTION:
/// Width and height are at the start of the file (header) and are separated by
/// commmas.  All other information is stored as single unsigned characters.

#include "stb_image.h"  // For reading images
#include "stb_image_write.h"

// This macro gets the length of a positive number (including zero).
#define getNumberLen(num) ( ((int)num<=0) ? 1 : (floor(log10(abs((int)num)))+1) )

void denseCompress(char*, char*);
void denseDecompress(char*, char*);
void writeFileWithLength(char*, char*&, long long);

// This is the constructor for the "simple" psudeo class.
int dense(char* inputFilepath, char* outputFilepath, char operation) {
    printf("Doing dense %c from %s to %s\n", operation, inputFilepath, outputFilepath);
    if (operation == 'c') {  // Case: compression.
        denseCompress(inputFilepath, outputFilepath);
    } else {  // Case: decompression.
        denseDecompress(inputFilepath, outputFilepath);
    }
    return 0; // This means good.
}

void denseCompress(char* inputFilepath, char* outputFilepath) {
    // Load the image.
    int width=0, height=0, bpp=0;
    unsigned char *rgb_image = stbi_load(inputFilepath, &width, &height, &bpp, 4);

    // Find length of output file string.
    long long outFileStringLength = 1;  // for \0
    int sizeHW = getNumberLen(height)+getNumberLen(width)+2;
    outFileStringLength += sizeHW;  // Include width and height.
    outFileStringLength += height*width*4;  // Include each character.

    //printf("length: %i\n", outFileStringLength);

    // Init the out file string with the height and width.
    char* outFileString = (char*) malloc( outFileStringLength * sizeof(char) );
    sprintf(outFileString, "%i,%i,", height, width);

    // Read the ascii characters from the png to string.
    long long fsPosition = sizeHW;
    long long index = 0;
    while (index < height*width*4) {
        // Add current number to string, by each individual character.
        outFileString[fsPosition] = (char) rgb_image[index];

        fsPosition++; index++;
    }

    stbi_image_free(rgb_image);

    char* outPtr = outFileString;
    writeFileWithLength(outputFilepath, outPtr, outFileStringLength);  // Write to the output filepath.  // WARNING: will overwrite file.
}

void denseDecompress(char* inputFilepath, char* outputFilepath) {
    FILE* fptr = fopen(inputFilepath, "rb+");

    if (fptr == NULL) {
        printf("Error: failed to read file.\n"); return;
    }

    unsigned char* rgb_image;  // Init the array of pixel values.

    // Height and Width grabbers.  "NO MONSTER IMAGE SIZES!"
    char chHeight[128];
    char chWidth[128];

    // Get height and width.
    int numberPos = 0;
    long long index = -2;
    int ch = fgetc(fptr);
    bool exitLoop = false;
    while( (ch != EOF) && (exitLoop == false) ) {
        if(index < 0) {  // case: reading width and height.
            if (ch == ',') {  // Case: add last numbers to string.
                index++;  // Go to next array position.
                numberPos = 0;
                if (index == 0) {  // Case: Finish reading header information.
                    exitLoop = true;
                }
            } else {  // Case: add current char to string.
                if (index == -2) {
                    chHeight[numberPos] = ch;
                    numberPos++;
                } else if (index == -1) {
                    chWidth[numberPos] = ch;
                    numberPos++;
                }
            }
        }
        ch = fgetc(fptr);
    }

    int height = atoi(chHeight);
    int width = atoi(chWidth);

    // Allocate space to the array that will contain the pixel values.
    rgb_image = (unsigned char*) malloc(height*width*4);

    // Seek to begining of data.
    fseek(fptr, getNumberLen(height)+getNumberLen(width)+2, SEEK_SET);

    // Read all data characters in the file.
    unsigned char currentByte[1];
    while(index < height*width*4) {
        fseek(fptr, getNumberLen(height)+getNumberLen(width)+2+index, SEEK_SET);  // Seek to next char  // ftell(fptr);
        //fread(currentByte, 1, 1, fptr);  // read one byte.
        //rgb_image[index] = (unsigned char) currentByte[0];
        rgb_image[index] = fgetc(fptr);

        index++;  // Go to next char.
        if(index < 200) {
            printf("#245.517: %i\n", index);  // This is a magic statement.  If this is not here, it doesn't work right.  Or at least, it used to.
            //printf("%i:%i |", (unsigned char) currentByte[0], ftell(fptr));
            fseek(fptr, getNumberLen(height)+getNumberLen(width)+2+index, SEEK_SET);  // Seek to next char  // ftell(fptr);
            //printf("%i : %i |", fgetc(fptr), ftell(fptr));
        }
    }

    fclose(fptr);  // Stop reading file.

    stbi_write_png(outputFilepath, width, height, 4, rgb_image, width*4);
    printf("File has been written to %s\n", outputFilepath);

    free(rgb_image);
}
