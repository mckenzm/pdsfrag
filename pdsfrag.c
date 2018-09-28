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
 *          (6)...Help text.
 *          (7)...man entry.
 *          (8)...Makefile.
 *          (9)...Zip packaging.
 *          (10)..Debian packaging.
 *          (11)..GitHub page update.
 *          (12)..Restrict to/Override allowed extensions list.
 *          (13)..Enumerate return codes as variables/macros.
 ****************************************************************************************/
#include <stdio.h>
#include <memory.h>
#include <ctype.h>

// declarations
FILE *inputFile
    ,*outputFile;

char *eofTest
    ,fileRecord[90]
    ,pdsMemberName[9]
    ,inputFileName[60]          // care to validate for too long.
    ,fileNameExtension[5] = "."
    ,control[9]           = ""
    ,outputFileName[13]   = "";

unsigned long count
             ,outputFilesCount = 0
             ,linesCount       = 0
             ,totalLinesCount  = 0
             ,linesRead        = 0;

// prototypes
void preamble              (void);
void testAndCloseOutputFile(void);
void testAndWriteRecord    (void);
int  testAndOpenNextFile   (void);

void preamble(void)
{
    printf("\n  pdsfrag - Decompose IEBPTPCH dump of partitioned dataset. \n");
    printf("  Copyright (c)(p) 2018 Matthew H. Mckenzie. \n\n");
}

void testAndCloseOutputFile(void)
{
    // Close previous output file. 1st time through do not close.
    // If $$$space then we did not open. Possibly could be a flag...
    if (strlen(outputFileName) && strncmp(outputFileName, "$$$space",8)!=0)
    {
        fclose(outputFile);
        printf(" (%lu lines)\n", linesCount);
        linesCount = 0;
    }
}

void testAndWriteRecord(void)
{
    if ((fileRecord[0] != 26) && strncmp(outputFileName, "$$$space",8) != 0)
    {
        // don't want eof nor anything inside $$$space
        fputs(fileRecord, outputFile);
        linesCount++;
        totalLinesCount++;
    }
}

int testAndOpenNextFile(void)
{
    // Check this very carefully, should be able to combine.
    // if $$$space then we do not want to open
    if (strncmp(outputFileName, "$$$space",8) != 0)
    {
        printf("  extracting: %s", outputFileName);
        outputFile = fopen(outputFileName, "w");

        if (outputFile == NULL)
        {
            printf("Error. Failed to open output file %s\n", outputFileName);
            return (5);
        }
        else
        {
            outputFilesCount++;
        }
    }

    return(0);
}

// **argv and *argv[] are equivalent
int main(int argn, char **argv)
{
    // get args for input file, fileNameExtension, sub-folder, validate
    if (argn != 3)
    {
        printf("Expected arguments (in this order) : [infile] [extension]\n");
        return 6;
    }

    if (strlen(argv[2]) > 4)
    {
        printf("fileNameExtension currently limited to 3 characters.\n");
        return 7;
    }

    preamble();
    strcpy(inputFileName,argv[1]);
    strcat(fileNameExtension, argv[2]);

    for (count = 0; count < strlen(fileNameExtension); count++)
    {
        fileNameExtension[count] = (char)tolower(fileNameExtension[count]);
    }

    // open input file
    inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL)
    {
        printf("Error. Failed to open input file %s\n", inputFileName);
        return (4);
    }

    // read a line
    eofTest = fgets(fileRecord, 82, inputFile);

    // while not at end....
    while (eofTest != NULL)
    {
        linesRead++;
        // test for header, in theory the control field could be the entire line, saving some processing.
        if (strncmp(fileRecord, "MEMBER NAME  ", 13) == 0)
        {
            // complementary span
            fileRecord[strcspn(fileRecord, "\r\n")] = 0;
            strcpy(pdsMemberName, &fileRecord[13]);

            // lowercase for 8.3 outputFileName
            for (count = 0; count < strlen(pdsMemberName); count++)
            {
                pdsMemberName[count] = (char)tolower(pdsMemberName[count]);
            }
            // changed?
            if (strcmp(pdsMemberName, control) != 0)
            {
                int rc;

                // close previous output file if we think there was one.
                testAndCloseOutputFile();

                // build the next output filename
                strcpy(outputFileName, pdsMemberName);
                strcat(outputFileName, fileNameExtension);

                // open the next output file if filename is acceptable
                rc = testAndOpenNextFile();
                if (rc) return(rc);
            }

            // save control field.
            strcpy(control, pdsMemberName);
        }
        else
        {
            if (linesRead == 1)
            {
                printf("  Error. Expected \"MEMBER NAME\" in first record of input file.\n\n");
                return (8);
            }
            testAndWriteRecord();
        }
        // read next line
        memset(fileRecord, 0, 81);
        eofTest = fgets(fileRecord, 80, inputFile);
    }   // end while

    // end processing
    testAndCloseOutputFile();
    fclose(inputFile);

    // print stats
    printf("\n  wrote %lu lines into %lu files.\n\n", totalLinesCount, outputFilesCount);

    return 0;
}
