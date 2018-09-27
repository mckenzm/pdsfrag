# pdsfrag
Small control-break program to extract PDS members from EIBPTPCH dumps.

Verbose by default, if you only want names (ala zip/unzip) use :

pdsfrag DEVJCL.txt JCL | grep extr |tr -s ' '|cut -d ' ' -f3

