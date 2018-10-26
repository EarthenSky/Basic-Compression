#include "cast.h"

/// DESCRIPTION:
/// Cast does some basic pattern compression and converts the 8 bits per channel
/// value into 7 bits per channel.
/// The pattern matching works by at the start of each row (for each channel),
/// having a 7-bit number, then unsigned 6-bit numbers (I saved them as 5-bit numbers
/// and 1 bit as a sign) detailing how much each pixel has changed from the last one.
/// The size of each section can be figured out by using height and width.

#include "stb_image.h"  // For reading images
#include "stb_image_write.h"

// This macro gets the length of a positive number (including zero).
#define getNumberLen(num) ( ((int)num<=0) ? 1 : (floor(log10(abs((int)num)))+1) )
#define absNumber(n) ( (n>0) ? n : -n )

void castCompress(char*, char*);
void castDecompress(char*, char*);
void writeFileWithLength(char*, char*&, long long);
void intToBinary(int, bool*, int);
unsigned char byteToChar(bool*&, int);

// This is the constructor for the "simple" psudeo class.
int cast(char* inputFilepath, char* outputFilepath, char operation) {
    printf("Doing cast %c from %s to %s\n", operation, inputFilepath, outputFilepath);
    if (operation == 'c') {  // Case: compression.
        castCompress(inputFilepath, outputFilepath);
    } else {  // Case: decompression.
        castDecompress(inputFilepath, outputFilepath);
    }
    return 0; // This means good.
}

void castCompress(char* inputFilepath, char* outputFilepath) {
    // Load the image.
    int width=0, height=0, bpp=0;
    unsigned char *rgb_image = stbi_load(inputFilepath, &width, &height, &bpp, 4);

    // Find header size.
    int headerSize = (int)getNumberLen(height)+(int)getNumberLen(width)+2;
    //char header[headerSize+1];
    //sprintf(header, "%i,%i,", height, width);

    // This array contains the binary contents of the file.
    long long filePosition = 0;
    long long fileSizeBinary = ((width-1)*6+7)*height*4;
    bool* fileBinary = (bool*) malloc( fileSizeBinary*sizeof(bool) );  // TODO: Use malloc?

    // Read r, g, b, then a, character values.  Save them to fileBinary.
    for(int offset=0; offset<4; offset++) {
        long long index = 0;
        int headValue=0;  // This values is the origin value of the current row.

        // Go through all pixel values for the current channel.
        while (index < height*width) {
            if( (index % width) == 0 ) {  // Case: first char in row. (7bit)
                // Convert 8-bit value into 7bit value.
                headValue = (int) rgb_image[(index*4)+offset] / 2;

                bool* binaryValue; intToBinary(headValue, binaryValue, 7);

                // Add the 7bit head value to the binary string.
                fileBinary[filePosition+0] = binaryValue[0];
                fileBinary[filePosition+1] = binaryValue[1];
                fileBinary[filePosition+2] = binaryValue[2];
                fileBinary[filePosition+3] = binaryValue[3];
                fileBinary[filePosition+4] = binaryValue[4];
                fileBinary[filePosition+5] = binaryValue[5];
                fileBinary[filePosition+6] = binaryValue[6];
                filePosition+=7;

                free(binaryValue);  // deallocate memory when done.
            }
            else {  // Case: another char in row. (6bit)
                // Get the step since the last value.
                int currentValue = (int)(rgb_image[(index*4)+offset] / 2) - headValue;

                // Limit the step to 5 bits.
                if(currentValue > 32)
                    currentValue = 32;
                else if(currentValue < -32)
                    currentValue = -32;

                // Update headValue.
                headValue += currentValue;

                // Get binary value as 5 bits.
                bool* binaryValue; intToBinary(absNumber(currentValue), binaryValue, 5);

                // 0 at the beginning of a number means it's positive, 1 is negitave.
                fileBinary[filePosition+0] = (currentValue > 0) ? 0 : 1;  // This bit is the sign.
                fileBinary[filePosition+1] = binaryValue[0];
                fileBinary[filePosition+2] = binaryValue[1];
                fileBinary[filePosition+3] = binaryValue[2];
                fileBinary[filePosition+4] = binaryValue[3];
                fileBinary[filePosition+5] = binaryValue[4];
                filePosition+=6;

                free(binaryValue);
            }
            index++;
        }
    }

    // Allocate string.
    char* outFileString = (char*) malloc( (fileSizeBinary/8)*sizeof(char)+headerSize );
    sprintf(outFileString, "%i,%i,", height, width);  // Add the header to the string.

    // Convert the bit array into 8-bit chars, then add to a string.
    for(long long i=0; i<fileSizeBinary; i+=8) {
        if (i > fileSizeBinary-8) {  // Case: last char, might need to add zeroes to complete the byte.
            // Fill last byte with 0s after the last binary values.
            bool* lastByte = (bool*) malloc( 8*sizeof(bool) );
            for(int j=0; j<8; j++) {
                if (j<fileSizeBinary-i)
                    lastByte[j] = fileBinary[i+j];
                else
                    lastByte[j] = 0;
            }

            outFileString[(i/8) + headerSize] = byteToChar(lastByte, 0);

            free(lastByte);
        }
        else {
            outFileString[(i/8) + headerSize] = byteToChar(fileBinary, i);
        }
    }

    // Free allocated memory ( outFileString gets freed in writeFileWithLength() )
    stbi_image_free(rgb_image);
    free(fileBinary);

    // Send the output string to be written.
    writeFileWithLength(outputFilepath, outFileString, (fileSizeBinary/8)*sizeof(char)+headerSize);  // Write to the output filepath.  // WARNING: will overwrite file.
}

// TODO: this
void castDecompress(char* inputFilepath, char* outputFilepath) {
    FILE* fptr = fopen(inputFilepath, "rb+");  // Read file in byte mode.

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

    int extraBinaryLen=0;  // This number is how many binary characters as useful.
    bool extraBinary[8];  // This array holds the binary characters.
    bool currentByte[8];

    unsigned char currentByte[1];

    // Read the entire file in binary sections.
    while(index < ((width-1)*6+7)*height*4) {
        fseek(fptr, getNumberLen(height)+getNumberLen(width)+2+index, SEEK_SET);  // Seek to next char  // ftell(fptr);
        if( (index % width) == 0 ) {  // Case: take 7 bits.
            bool usefulBits[7];  // This holds the actual pixel value retrived.

            intToBinary(fgetc(fptr), currentByte, 8);  // Grab 8 bits.

            // Take the saved numbers from extraBinary.
            int index = 0;
            while (extraBinaryLen > 0) {
                usefulBits[index] = extraBinary[index];

                extraBinaryLen--; index++;
            }

            // Put the new numbers into usefulBits.

            // Put the leftover numbers in extraBinary.

        } else {  // Case: take 1 and 5 bits.
            bool sign;
            bool usefulByte[5];  // This holds the actual pixel value retrived.

        }

        // Cut the binary from the array at the right anmounts and convert to number.

        // Convert number from step / 7-bits into an 8-bit pixel value.

        // Add the pixel value at the correct position in the file (r, g, b then a)
        //rgb_image[index] = fgetc(fptr);

        index++;  // Go to next 8-bits.
    }

    fclose(fptr);  // Stop reading file.

    stbi_write_png(outputFilepath, width, height, 4, rgb_image, width*4);
    printf("File has been written to %s\n", outputFilepath);

    free(rgb_image);
}
