      *-------------------------------------------------------------*
      *  DRESSPRT : Dresses "print" output from IEBPTPCH after dump *
      *             of an entire PDS to remove page lines and ANSI  *
      *             column. ANS COBOL (1972)                        *
      *-------------------------------------------------------------*
       IDENTIFICATION DIVISION.
      *------------------------*
       PROGRAM-ID.  'DRESSPRT'.

       ENVIRONMENT DIVISION.
      *---------------------*
       CONFIGURATION SECTION.

       SOURCE-COMPUTER.  IBM-360.
       OBJECT-COMPUTER.  IBM-360.

       SPECIAL-NAMES.
           CONSOLE IS CNSL.

       INPUT-OUTPUT SECTION.
      *---------------------*    INFIL OUTFIL
       FILE-CONTROL.
           SELECT INFILE  ASSIGN TO UT-S-INFIL.
           SELECT OUTFILE ASSIGN TO UT-S-OUTFIL.


       DATA DIVISION.
      *--------------*           matching records per block
       FILE SECTION.
       FD  INFILE
           RECORDING MODE IS F
           RECORD CONTAINS  81 CHARACTERS
           BLOCK  CONTAINS 100 RECORDS
           LABEL RECORDS ARE OMITTED
           DATA RECORD IS INREC.
       01  INREC               PIC X(81).

       FD  OUTFILE
           RECORDING MODE IS F
           RECORD CONTAINS  80 CHARACTERS
           BLOCK  CONTAINS  49 RECORDS
           LABEL RECORDS ARE OMITTED
           DATA RECORD IS OUTREC.
       01  OUTREC              PIC X(80).


       WORKING-STORAGE SECTION.
      *------------------------*
       01  SW-SWITCHES.
           05 END-OF-FILE-IND     PIC X VALUE SPACE.
              88 SW-END-OF-FILE         VALUE 'Y'.

       01  CH-VARIABLES.
           05 CH-OVERLAY.
              10 CH-PRT-CTL       PIC X.
                 88 SW-NORMAL           VALUE SPACE.
                 88 SW-OVERSTRIKE       VALUE '+'.
                 88 SW-SKIP2            VALUE '-'.
                 88 SW-SKIP1            VALUE '0'.
                 88 SW-FORMFEED         VALUE '1'.
              10 CH-PAYLOAD       PIC X(80).


       PROCEDURE DIVISION.
      *-------------------*
       00-MAINLINE.
           PERFORM 10-SETUP THROUGH 10-EXIT
           PERFORM 20-LOOP  THROUGH 20-EXIT
              UNTIL SW-END-OF-FILE.
           PERFORM 30-FINISH.

      *                          ANS COBOL (1972) NO FILE STATUS.
       10-SETUP.
           OPEN INPUT  INFILE
           OPEN OUTPUT OUTFILE
           READ INFILE INTO CH-OVERLAY AT END PERFORM 30-FINISH.

       10-EXIT.
           EXIT.

      *                          do not write ANSI/Page line.
       20-LOOP.
           IF NOT SW-FORMFEED
              WRITE OUTREC FROM CH-PAYLOAD.
           READ INFILE INTO CH-OVERLAY
              AT END MOVE 'Y' TO END-OF-FILE-IND.

       20-EXIT.
           EXIT.

      *                         typical housekeeping.
       30-FINISH.
           CLOSE INFILE OUTFILE
           STOP RUN.
