/****************************************************************************************
*   pdsfrag - split iebptpch pds dump  a.k.a. "ptpch".
*
*   Matthew H. McKenzie 2018
*
*   To do:
*          (1)...Use a flag to for $$$space status.
*          (2)...Standardise argument handling with switches.
*          (3)...Quiet and Verbose modes.
*          (4)...Doco in flowerbox.
*          (5)...Flowerbox coments.
*          (6)...Expand statistics for individual record counts ala "wc -l *.jcl".
*          (7)...Help text.
*          (8)...man entry.
*          (9)...Makefile.
*          (10)..Zip packaging.
*          (11)..Debian packaging.
*          (12)..GitHub page update.
*          (13)..Restrict to/Override allowed extensions list.
****************************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <ctype.h>

// declarations
FILE *inputFile;
FILE *outputFile;
char fileRecord[90];
char *eofTest;
char member[9];
char infileName[60];            // care to validate for too long.
char extension[5]=".";
char control[9];
char filename[13] = "";
unsigned long count, filesCount = 0, linesCount = 0, totalLinesCount = 0;

// prototypes
void preamble(void);
void testAndCloseOutputFile(void);

void preamble(void)
{
    printf("\npdsfrag - Decompose IEBPTPCH dump of partitioned dataset. \n");
    printf("Copyright (c)(p) 2018 Matthew H. Mckenzie. \n\n");
}

void testAndCloseOutputFile(void)
{
    // Close previous output file. 1st time through do not close. 
    // If $$$space then we did not open. Possibly could be a flag...
    if (strlen(filename) && strncmp(filename, "$$$space",8)!=0)
    {
        fclose(outputFile);
        printf(" %lu records written.\n", linesCount);
        linesCount = 0;
    }
}

// **argv and *argv[] are equivalent
int main(int argn, char **argv)
{
    // get args for input file, extension, sub-folder, validate
    if (argn != 3)
    {
        printf("Expected arguments (in this order) : [infile] [extension]\n");
        return 6;
    }

    if (strlen(argv[2]) > 4)
    {
        printf("extension currently limited to 3 characters.\n");
        return 7;
    }

    preamble();
    strcpy(infileName,argv[1]);
    strcat(extension, argv[2]);

    for (count = 0; count < strlen(extension); count++)
    {
        extension[count] = (char)tolower(extension[count]);
    }

    // open input file
    inputFile = fopen(infileName, "r");
    if (inputFile == NULL)
    {
        printf("Error. Failed to open input file %s\n", infileName);
        return (4);
    }

    // read a line
    eofTest = fgets(fileRecord, 82, inputFile);

    // while not at end....
    while (eofTest != NULL)
    {
        // test for header, in theory the control field could be the entire line, saving some processing.
        if (memcmp(fileRecord, "MEMBER NAME  ", 13) == 0)
        {
            // complementary span
            fileRecord[strcspn(fileRecord, "\r\n")] = 0;
            strcpy(member, &fileRecord[13]);

            // lowercase for 8.3 filename
            for (count = 0; count < strlen(member); count++)
            {
                member[count] = (char)tolower(member[count]);
            }
            // changed?
            if (strcmp(member, control) != 0)
            {
                // close previous output file if we think there was one.
                testAndCloseOutputFile();
                strcpy(filename, member);
                strcat(filename, extension);

                // Check this very carefully, should be able to combine.
                // if $$$space then we do not want to open
                if (strncmp(filename, "$$$space",8) != 0)
                {
                    printf("Extracting %-13s", filename);
                    outputFile = fopen(filename, "w");

                    if (outputFile == NULL)
                    {
                        printf("Error. Failed to open output file %s\n", filename);
                        return (5);
                    }
                    else
                    {
                        filesCount++;
                    }
                }
            }
            // save control field.
            strcpy(control, member);
        }
        else if ((fileRecord[0] != 26) && strncmp(filename, "$$$space",8) != 0)
        {
            // don't want eof nor anything inside $$$space
            fputs(fileRecord, outputFile);
            linesCount++;
            totalLinesCount++;
        }
        // read next line
        memset(fileRecord, 0, 81);
        eofTest = fgets(fileRecord, 80, inputFile);
    }                                           // end while

    // end processing
    testAndCloseOutputFile();
    fclose(inputFile);

    // print stats
    printf("\nWrote %lu records in %lu files.\n\n", totalLinesCount, filesCount);

    return 0;
}
