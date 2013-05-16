#!/bin/sh
if ! perm=`mod_find accounts:permissions`; then
	echo "This script requires Kraknet to run. Check your \$PATH."
	exit
fi

if [ -z "$perm" -o "$perm" -ne 0 ]; then
	echo "<meta http-equiv=\"refresh\" content=\"0;/\">"
fi
