# pdsfrag
Small control-break program to extract PDS members from EIBPTPCH dumps.

Verbose by default, if you only want names (ala zip/unzip) use :

pdsfrag DEVJCL.txt -eJCL | grep extr |tr -s ' '|cut -d ' ' -f3

At this time, all members are extracted, a future switch may allow extraction by name or pattern.

