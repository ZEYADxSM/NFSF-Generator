//place for libraries

#define MAX_LINE_LENGTH 1000
#define MAX_TOKEN_LENGTH 50
#define MAX_TRANSFORMS 100
#define MAX_GRAPHICS 100
#define MAX_BRANCHES 100


// Structure definitions for Pair, Transform, Graphic, Branch...
// Structure to represent a pair of values (x, y)
typedef struct {
    double x;
    double y;
} Pair;

// Structure to represent a transformation
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    double rotation;
    Pair translation;
    double scale;
} Transform;

// Structure to represent a graphic item
typedef struct {
    char name[MAX_TOKEN_LENGTH];
    Pair coordinates[MAX_TOKEN_LENGTH];
    int numCoordinates;
} Graphic;

// Structure to represent a fractal branch
typedef struct {
    Transform transform;
    char type[MAX_TOKEN_LENGTH];  // "GRAPHIC" or "FRACTAL"
    char refName[MAX_TOKEN_LENGTH];
    Pair range;
} Branch;

// Function to parse NFSF file and fill data structures
void parseNFSFFile(const char *filename, Transform *transforms, Graphic *graphics, Branch *branches, int *numTransforms, int *numGraphics, int *numBranches);
// Function to generate SVG file from parsed data
void generateSVGFile(const char *filename, Transform *transforms, Graphic *graphics, Branch *branches, int numTransforms, int numGraphics, int numBranches);


// Function declarations
void applyTransform(Pair *point, Transform transform);
void rotateCoordinate(Pair *point, double angle);
void scaleCoordinate(Pair *point, double scale);
void translateCoordinate(Pair *point, Pair translation);

int main(int argc, char *argv[]) {
    char inputFilename[MAX_TOKEN_LENGTH];

    if (argc != 1) {
        fprintf(stderr, "Usage: %s \n", argv[0]);
        return EXIT_FAILURE;
    }

    // Ask the user for the input filename
    printf("Enter the input NFSF file name: ");
    scanf("%s", inputFilename);

    // Create the output filename by appending ".svg"
    char outputFilename[MAX_TOKEN_LENGTH];
    strcpy(outputFilename, inputFilename);
    strcat(outputFilename, ".svg");

    Transform transforms[MAX_TRANSFORMS];
    Graphic graphics[MAX_GRAPHICS];
    Branch branches[MAX_BRANCHES];
    int numTransforms = 0, numGraphics = 0, numBranches = 0;

    parseNFSFFile(inputFilename, transforms, graphics, branches, &numTransforms, &numGraphics, &numBranches);
    generateSVGFile(outputFilename, transforms, graphics, branches, numTransforms, numGraphics, numBranches);

    return EXIT_SUCCESS;
}

// parseNFSFFile function
void parseNFSFFile(const char *filename, Transform *transforms, Graphic *graphics, Branch *branches, int *numTransforms, int *numGraphics, int *numBranches) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error opening NFSF file for reading.\n");
        exit(EXIT_FAILURE);
    }

    char line[MAX_LINE_LENGTH];
    char keyword[MAX_TOKEN_LENGTH];

// skipping comments in the NFSF text file
    while (fgets(line, sizeof(line), file)) {
        // Ignore comments
        if (line[0] == '/' && line[1] == '/') {
            continue;
        }

        // Tokenize the line to get the first keyword
        if (sscanf(line, "%s", keyword) != 1) {
            continue; // Skip lines without a keyword
        }

        // Parse based on the detected keyword
        if (strcmp(keyword, "TRANSFORM") == 0) {
            // Parse TRANSFORM instruction
            sscanf(line, "%*s %s ROTATION %lf TRANSLATION (%lf,%lf) SCALE %lf",
                   transforms[*numTransforms].name,
                   &transforms[*numTransforms].rotation,
                   &transforms[*numTransforms].translation.x,
                   &transforms[*numTransforms].translation.y,
                   &transforms[*numTransforms].scale);
            (*numTransforms)++;

        } else if (strcmp(keyword, "GRAPHIC") == 0) {
            // Parse GRAPHIC instruction
            sscanf(line, "%*s %s", graphics[*numGraphics].name);

            // Read coordinates until a newline is encountered
            int i = 0;
            while (fgets(line, sizeof(line), file) && line[0] != '\n') {
                sscanf(line, "%lf,%lf", &graphics[*numGraphics].coordinates[i].x, &graphics[*numGraphics].coordinates[i].y);
                i++;
            }

            graphics[*numGraphics].numCoordinates = i;
            (*numGraphics)++;
        } else if (strcmp(keyword, "FRACTAL") == 0) {
            // Parse FRACTAL instruction
            sscanf(line, "%*s %s", branches[*numBranches].refName);
            strcpy(branches[*numBranches].type, "FRACTAL");
            (*numBranches)++;
        } else if (strcmp(keyword, "BRANCH") == 0) {

            // Parse BRANCH instruction
            sscanf(line, "%*s %s [%lf:%lf] %s %s",
                   branches[*numBranches].transform.name,
                   &branches[*numBranches].range.x,
                   &branches[*numBranches].range.y,
                   branches[*numBranches].type,
                   branches[*numBranches].refName);
            (*numBranches)++;
        }
    }

    fclose(file);
}

void applyTransform(Pair *point, Transform transform) {

    // Apply scale first, then rotate, and finally translate
    scaleCoordinate(point, transform.scale);
    rotateCoordinate(point, transform.rotation);
    translateCoordinate(point, transform.translation);
}

void rotateCoordinate(Pair *point, double angle) {
    double x = point->x;
    double y = point->y;
    point->x = x * cos(angle) - y * sin(angle);
    point->y = x * sin(angle) + y * cos(angle);
}

void scaleCoordinate(Pair *point, double scale) {
    point->x *= scale;
    point->y *= scale;
}

void translateCoordinate(Pair *point, Pair translation) {
    point->x += translation.x;
    point->y += translation.y;
}

void generateSVGFile(const char *filename, Transform *transforms, Graphic *graphics, Branch *branches, int numTransforms, int numGraphics, int numBranches) {
    FILE *svgFile = fopen(filename, "w");
    if (!svgFile) {
        fprintf(stderr, "Error opening SVG file for writing.\n");
        exit(EXIT_FAILURE);
    }

    // Write SVG header
    fprintf(svgFile, "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n");
    fprintf(svgFile, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" ""\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
    fprintf(svgFile, "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"700\" height=\"700\">\n");


    // Apply transformations and write SVG polyline for each graphic
    for (int i = 0; i < numGraphics; ++i) {
        fprintf(svgFile, "<polyline points=\"");

        for (int j = 0; j < graphics[i].numCoordinates; ++j) {
            Pair transformedPoint = graphics[i].coordinates[j];
            for (int k = 0; k < numTransforms; ++k) {
                applyTransform(&transformedPoint, transforms[k]);
            }

            // Reverse the vertical axis to display in conventional Cartesian coordinates
            transformedPoint.y = -transformedPoint.y;

            fprintf(svgFile, "%.2lf,%.2lf ", transformedPoint.x, transformedPoint.y);
        }

        fprintf(svgFile, "\" style=\"fill:none;stroke:black;stroke-width:1.0\"/>\n");
    }

    // Write SVG footer
    fprintf(svgFile, "</svg>\n");

    fclose(svgFile);
    printf("SVG file generated successfully.\n");
}
