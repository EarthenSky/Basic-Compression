#include "simple.h"

/// This compression method is a baseline.  It is probably the worst you could
/// do if you actually tried.
/// This method breaks each individual pixel up into it's four components and
/// separates each number with a comma.
/// The height and width of the image are the first two numbers, respectively.

// This macro gets the length of a positive number (including zero).
#define getNumberLen(num) ( ((int)num<=0) ? 1 : (floor(log10(abs((int)num)))+1) )

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"  // For reading images
#endif

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif

void simpleCompress(char*, char*);
void simpleDecompress(char*, char*);
void writeFile(char*, char*&);

// This is the constructor for the "simple" psudeo class.
int simple(char* inputFilepath, char* outputFilepath, char operation) {
    printf("Doing simple %c from %s to %s\n", operation, inputFilepath, outputFilepath);
    if (operation == 'c') {  // Case: compression.
        simpleCompress(inputFilepath, outputFilepath);
    } else {  // Case: decompression.
        simpleDecompress(inputFilepath, outputFilepath);
    }
    return 0; // This means good.
}

void simpleCompress(char* inputFilepath, char* outputFilepath) {
    // Load the image.
    int width=0, height=0, bpp=0;
    unsigned char *rgb_image = stbi_load(inputFilepath, &width, &height, &bpp, 4);

    // Find length of output file string.
    long long outFileStringLength = 0;  // 0 becasue there is no last comma, but there is \0.
    int sizeHW = getNumberLen(height)+getNumberLen(width)+2;
    outFileStringLength += sizeHW;  // Include width and height.
    for (long long i=0; i<height*width*4; i++) {  // Find length of all numbers & commas.
        outFileStringLength += getNumberLen(rgb_image[i])+1;  //+1 for each comma.
    }

    // Init the out file string with the height and width.
    //char outFileString[outFileStringLength];
    char* outFileString = (char*) malloc( outFileStringLength * sizeof(char) );
    sprintf(outFileString, "%i,%i,", height, width);

    // Read the ascii characters to a string.
    long long fsPosition = sizeHW;
    long long index = 0;
    while (index < height*width*4) {
        // Convert number to string.
        int numStrLen = getNumberLen(rgb_image[index]);
        char numStr[numStrLen];
        itoa(rgb_image[index], numStr, 10);

        // Add current number to string, by individual characters.
        int ci = 0;
        while(ci < numStrLen) {  //-1 to not use the \0.
            outFileString[fsPosition+ci] = numStr[ci];
            ci++;
        }

        outFileString[fsPosition+ci] = ',';  // Add the comma.

        fsPosition += numStrLen+1;
        index++;
    }
    outFileString[outFileStringLength-1] = '\0';

    stbi_image_free(rgb_image);

    // Remove file-end from filepath
    //char* cutPtr;
    //cutPtr = strrchr(filepath, '.');
    //if (cutPtr != NULL) { *cutPtr = '\0'; }

    // Add new file ending.
    //char newFilepath[strlen(filepath)+8]; strcpy(newFilepath, filepath);
    //newFilepath[strlen(filepath)] = '.'; newFilepath[strlen(filepath)+1] = 's';
    //newFilepath[strlen(filepath)+2] = 'i'; newFilepath[strlen(filepath)+3] = 'm';
    //newFilepath[strlen(filepath)+4] = 'p'; newFilepath[strlen(filepath)+5] = 'l';
    //newFilepath[strlen(filepath)+6] = 'e'; newFilepath[strlen(filepath)+7] = '\0';

    char* outPtr = outFileString;
    writeFile(outputFilepath, outPtr);  // Write to the output filepath.  // WARNING: will overwrite file.
}

void simpleDecompress(char* inputFilepath, char* outputFilepath) {
    FILE* fptr = fopen(inputFilepath, "r");

    if (fptr == NULL) {
        printf("Error: failed to read file.\n"); return;
    }

    uint8_t* rgb_image;  // Init the array of pixel values.

    // Height and Width grabbers.  "NO MONSTER IMAGE SIZES!"
    char chHeight[128];
    char chWidth[128];

    //long long fileLength = 0;
    char number[4]; int numberPos = 0;
    long long index = -2;
    char ch;
    while((ch = fgetc(fptr)) != EOF) {
        if(index < 0) {  // case: reading width and height.
            if (ch == ',') {  // Case: add last numbers to string.
                index++;  // Go to next array position.
                numberPos = 0;
                if (index == 0) {  // Case: Finish reading header information.
                    rgb_image = (uint8_t*) malloc(atoi(chWidth)*atoi(chHeight)*4);  // Allocate space to the array that will contain the pixel values.
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
        else {
            if (ch == ',') {  // Case: add last numbers to string.
                rgb_image[index] = (uint8_t) atoi(number);
                index++;  // Go to next array position.
                strcpy(number, "   \0");
                numberPos = 0;
                //(index % 99 == 0) ? printf("number %c\n", number[2]) : 0;  // DEBUG: this.
            } else {  // Case: add current char to string.
                number[numberPos] = ch;
                numberPos++;
            }
        }
    }

    fclose(fptr);

    int height = atoi(chHeight);
    int width = atoi(chWidth);

    // Remove file-end from filepath
    //char* cutPtr;
    //cutPtr = strrchr(filepath, '.');
    //if (cutPtr != NULL) { *cutPtr = '\0'; }

    // Add new file ending.
    //char newFilepath[strlen(filepath)+5]; strcpy(newFilepath, filepath);
    //newFilepath[strlen(filepath)] = '.'; newFilepath[strlen(filepath)+1] = 'p';
    //newFilepath[strlen(filepath)+2] = 'n'; newFilepath[strlen(filepath)+3] = 'g';
    //newFilepath[strlen(filepath)+4] = '\0';

    stbi_write_png(outputFilepath, width, height, 4, rgb_image, width*4);

    printf("File has been written to %s\n", outputFilepath);

    free(rgb_image);
}
