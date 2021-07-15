# pdsfrag
Small control-break program to extract PDS members from EIBPTPCH dumps.

Verbose by default, if you only want names (ala zip/unzip) use :

```pdsfrag DEVJCL.txt -eJCL | grep extr |tr -s ' '|cut -d ' ' -f3```

At this time, all members are extracted, a future switch may allow extraction by name or pattern.

Compile with : ```gcc -O3 -march=native -opdsfrag pdsfrag.c```

Install with : ```sudo cp pdsfrag /usr/bin```

Hercules and TK4- users can do the tailoring of print output in bash :
```cat dump.jcl | grep -v ^1 | cut -c2-73 > bulk.jcl && ./pdsfrag bulk.jcl -e jcl ```

IEBGENER cards that merely slice the file after column 1, a crude conversion of FBA to FB :

'''//SYSIN    DD  *
  GENERATE MAXFLDS=1
  RECORD FIELD=(132,2,,1)
/* '''


Or we can have a default to skip those lines in processing ?  (on the to-do list).
