/****************************************************************************************
 *   pdsfrag - split IEBPTPCH PDS dump a.k.a. "PTPCH".
 *
 *   Matthew H. McKenzie 2018                    https://github.com/mckenzm/pdsfrag
 *
 *   To do:
 *          ...Use a flag for $$$space status, or a suppression flag.
 *          ...man entry.
 *          ...Makefile.
 *          ...Zip packaging.
 *          ...Debian packaging.
 *          ...Enumerate return codes as variables/macros.
 *          ...Auto detect print control characters or MEMBER NAME offset.
 *          ...Header file.
 *
 *   Longer term:
 *          ...Glob for input files, extension affinity.
 *          ...Switch for probe to guess extension ala ISPF edit HILITE.
 *          ...Scripted execution (but currently trivial to repeatedly call.
 *          ...Gui Interface with GTK or similar.
 *          ...EOL conversion. DOS/Unix/Mac, again trivial to post process.
 *
 ****************************************************************************************/
typedef enum { false, true } bool;

#define ENDED_OK               0
#define FILE_NOT_FOUND         1
#define FILE_NOT_SUPPLIED      2
#define EXTENSION_OVERLENGTH   3
#define INVALID_EXTENSION_DOT  4
#define INVALID_FILE           5
#define OPEN_FOR_WRITE_FAIL    6
#define BAD_OPTION            99

//includes
#include <stdio.h>
#include <memory.h>
#include <ctype.h>
#include <unistd.h>

// declarations
FILE *inputFile
    ,*outputFile;

char *eofTest
    ,fileRecord[90]
    ,pdsMemberName[9]
    ,inputFileName[60]                  // care to validate for too long.
    ,fileNameExtension[5] = "."
    ,controlField[9]      = ""
    ,outputFileName[13]   = "";

unsigned long count
             ,outputFilesCount = 0
             ,linesCount       = 0
             ,totalLinesCount  = 0
             ,linesRead        = 0;

bool flagQuiet             = false     // for those too lazy to pipe to grep...
    ,flagExtensionSupplied = false
    ,flagNoExtension       = false;

// prototypes
void preamble              (void);
void testAndCloseOutputFile(void);
void testAndWriteRecord    (void);
int  testAndOpenNextFile   (void);
void helpText              (void);

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
            printf("  Error. Failed to open output file %s\n\n", outputFileName);
            return (OPEN_FOR_WRITE_FAIL);
        }
        else
        {
            outputFilesCount++;
        }
    }

    return(ENDED_OK);
}

void helpText(void)
{
    printf("\n  Usage : pdsfrag input-file [-e extension] [-q] \n\n");

    printf("  e.g. pdsfrag JCLDUMP.txt -e jcl \n\n");

    printf("  -h     help, this text.\n");
    printf("  -e     up to 3 character filename extension, else .mbr will be used.\n");
    printf("  -n     no extension, overrides -e.\n");
    printf("  -q     quieter, no header or footer. \n\n");

    printf("  This utility is intended to read a file produced by the JCL described at :\n");
    printf("  http://mainframewizard.com/content/jcl-unload-all-members-pds-ps-flat-file\n\n");

    printf("  i.e. IEBPTPCH has been used to dump PDS members to a Physical Sequential file,\n");
    printf("  and thereafter the leading character has been dropped, and the file has been \n");
    printf("  transferred to a PC. Example JCL can be found on the GitHub page, along with \n");
    printf("  test files.\n\n");

    printf("  There are no plans to automatically detect reading of the raw file of LRECL\n");
    printf("  up to 81, but extension should be trivial.\n\n");

    printf("  This utility does not write anything for any $$$SPACE member, nor does it\n");
    printf("  write any Ctrl-Z EOF character create by IND$FILE transfers. \n\n");

}

// **argv and *argv[] are equivalent
int main(int argc, char **argv)
{
    char **positionals;

    for (;;)
    {
        int opt = getopt(argc, argv, "qHhne:");
        if (opt == -1) break;

        switch (opt)
        {
            case 'e':
                flagExtensionSupplied = true;
                if (strlen(optarg) > 3)
                {
                    printf("fileName extension currently limited to 3 characters.\n");
                    return(EXTENSION_OVERLENGTH);
                }
                else
                {
                    strcat(fileNameExtension, optarg);
                }
                break;
            case 'h':
            case 'H':
                helpText();
                return(ENDED_OK);
            case 'q':
                flagQuiet = true;
                break;
            case 'n':
                flagNoExtension = true;
                break;
            default:
                helpText();
                return(BAD_OPTION);
        }
    }

    positionals = &argv[optind];
    for (; *positionals; positionals++)
    {
        if (strlen(inputFileName) == 0)
        {
           strcpy(inputFileName,*positionals);
        }
        else
        {
            helpText();
            return(BAD_OPTION);
        }
    }

    // further Mickey Mouse validation
    if (strlen(inputFileName) == 0)
    {
        printf("\n  error. Missing input file name.\n\n");
        return(FILE_NOT_SUPPLIED);
    }

    if (flagNoExtension == true)
    {
        fileNameExtension[0] = 0;
    }
    else
    {
        if (flagExtensionSupplied == false)
        {
            strcpy(fileNameExtension,".mbr");
        }
    }

    if (strncmp(fileNameExtension, "..", 2) == 0)
    {
        printf("\n  error. extension may not begin with '.'.\n\n");
        return(INVALID_EXTENSION_DOT);
    }

    for (count = 0; count < strlen(fileNameExtension); count++)
    {
        fileNameExtension[count] = (char)tolower(fileNameExtension[count]);
    }

    /************************************************************************************
     * Main Processing starts here
     ************************************************************************************/

    if (flagQuiet == false)
    {
        preamble();
    }

    // open input file
    inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL)
    {
        printf("  Error. Failed to open input file %s\n", inputFileName);
        return(FILE_NOT_FOUND);
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
            if (strcmp(pdsMemberName, controlField) != 0)
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
            strcpy(controlField, pdsMemberName);
        }
        else
        {
            if (linesRead == 1)
            {
                printf("  Error. Expected \"MEMBER NAME\" in first record of input file.\n\n");
                return(INVALID_FILE);
            }
            testAndWriteRecord();
        }
        // read next line
        eofTest = fgets(fileRecord, 80, inputFile);
    }   // end while

    // end processing
    testAndCloseOutputFile();
    fclose(inputFile);

    // print stats
    if (flagQuiet == false)
    {
        printf("\n  wrote %lu lines into %lu files.\n\n", totalLinesCount, outputFilesCount);
    }

    return(ENDED_OK);
}
