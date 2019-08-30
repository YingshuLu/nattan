#!/bin/sh

MIMETYPES=mime.types
FILE=../mimes.ini
APACHE_MIMES=https://svn.apache.org/repos/asf/httpd/httpd/trunk/docs/conf/mime.types

rm -f $MIMETYPES
wget $APACHE_MIMES
echo "[mimes]" > $FILE
cat $MIMETYPES | grep -v "#" | awk '{n=split($0,a," "); for(i=2;i<=n; ++i) print a[i]"="a[1]}' >> $FILE
