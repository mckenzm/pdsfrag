#!/usr/bin/env python3
""" pdsfrag - to split a IEBPTPCH file (non FBA) into 8.3 named members.
    Crude, no statistics yet.
"""
import sys

extension = '.jcl'      # default extension
infile    = "bulk.jcl"  # default input file
saved_name = ''         # saved control value

if len(sys.argv) > 1:
    infile = sys.argv[1]

if len(sys.argv) > 2:
    extension = '.' + sys.argv[2].lower()

with open(infile, "r") as fd1:
     for line in fd1:

         # May want to consider coping with FBA
         if line[0:11] == "MEMBER NAME":
             words = line.split()

             # Test for break (not a function)  
             if words[2] != saved_name:

             # Do not try to close on break the first time.
                 if saved_name != '':
                     fd2.close()

                 # Save new value for comaprison and open file.
                 saved_name = words[2]
                 outfile    = words[2].lower() + extension
                 fd2        = open(outfile,"w")

         else:

             # ordinary line of file.
             if saved_name != '':    # file should be open
                 fd2.write(line)

# close output file if there was one.
if saved_name != '':
    fd2.close()
