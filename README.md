# Basic-Compression

These are just some basic compression algorithms, they are not designed to be efficient but to help me better understand compression.

#### simple
This compression method is a baseline.  It is probably the worst you could do if you actually tried. This method breaks each individual pixel up into it's four components and separates each number with a comma. The height and width of the image are the first two numbers, respectively.

#### dense
Width and height are at the start of the file (header) and are separated by commmas.  All other information is stored as single unsigned characters.
