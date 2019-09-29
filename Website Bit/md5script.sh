#!/bin/bash
CHECKSUMFILENAME="latestFiles/checksumFile.a"
md5AndStuff=`find latestFiles/* -type f -exec bash -c 'x=$(md5sum $0 2>/dev/null);x=${x%"  $0"};printf "$x###${0#'latestFiles/'}\n";' {} \;`

rm -rf $CHECKSUMFILENAME
echo $md5AndStuff >> $CHECKSUMFILENAME
echo $md5AndStuff