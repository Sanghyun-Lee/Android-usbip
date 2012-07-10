#!/bin/sh

## Delete a cscope.files
[ "$1" = "-r" -o "$1" = "-R" ] && rm -f cscope.files > /dev/null


## Make a cscope.files
if [ ! -f cscope.files ]; then
    echo -n "Making cscope.files ......... : "
    find . \( -name .svn -o -name CVS -o -name .git \) -prune -o \
    \( -name '*.java' -o -name '*.mk' -o -name '*.aidl' -o -name '*.xml' -o \
	-name '*.sh'  -o -name '*.CPP' -o -name '*.cpp' -o -name '*.C' -o -name '*.c' -o -name '*.H' -o -name '*.h' -o -name '*.HPP' -o -name '*.hpp' -o -name '*.s' -o -name '*.S' \) \
    -print > cscope.files

    if [ $? = 0 ] ; then
        echo "[ OK ]"
    else
        echo "[ Failure ]"
    fi  
fi


## Exit when cscope.files size is 0
if [ ! -s cscope.files ];then
    echo "Target files are not exist."
    rm -f cscope.files
    exit 1
fi


## Delete a database files (cscope.out, tags)
if [ -f cscope.out -o -f tags ]; then
    rm -f cscope.out tags
fi


## Make a cscope.out
    echo -n "Making cscope.out ........... : "
cscope -h > /dev/null 2>&1
if [ $? -eq 0 ];then
    cscope -U -b -i cscope.files
    echo "[ OK ]"
else
    echo "[ Failure ]"
fi


## Make tags
    echo -n "Making tags ................. : "
ctags -L cscope.files
if [ $? = 0 ] ; then
    echo "[ OK ]"
else
    echo "[ Failure ]"
fi


echo "done"
