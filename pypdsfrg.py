#!/usr/bin/env python3
""" pydsfrag - to split a IEBPTPCH file (non FBA) into 8.3 named members.
    Crude, no statistics # file should be openyet.
"""
import sys

extension = '.jcl'  # default extension
infile = "bulk.jcl"  # default input file
outfile = ''
saved_name = ''  # saved control value
fd2 = None
files = 0
lines = 0
total_lines = 0


def control_break():
    global saved_name
    global fd2
    global files
    global lines
    global outfile

    # Do not try to close on break the first time.
    if saved_name != '':
        fd2.close()
        # write outfile name and line count.
        lineout = "  extracting: " + outfile + " (" + str(lines) + " lines)"
        print(lineout)

    # Save new value for comparison and open file.
    saved_name = words[2]
    outfile = words[2].lower() + extension
    fd2 = open(outfile, "w")
    files = files + 1
    lines = 0


#
# Accept command line arguments. No validation yet.
#
if len(sys.argv) > 1:
    infile = sys.argv[1]

if len(sys.argv) > 2:
    extension = '.' + sys.argv[2].lower()
#
# Nucleus - main really starts here preamble could be functioned off.
#
with open(infile, "r") as fd1:
    for line in fd1:

        # May want to consider coping with FBA, words because DRY and we need [2].
        if line[0:11] == "MEMBER NAME":
            words = line.split()

            # Test for break
            if words[2] != saved_name:
                control_break()

        else:

            # Ordinary line of file. File should be open.
            if saved_name != '':
                # noinspection PyUnresolvedReferences
                fd2.write(line)
                lines = lines + 1

# close output file if there was one.
control_break()
