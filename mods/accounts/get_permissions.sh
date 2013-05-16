#!/bin/bash

if test -z "$kraknet_user"; then
	echo -n "-1"
	exit
fi

perm=`echo "select permission_level from users where username=\"$kraknet_user\";"|sqlite3 /home/ghandi/db/bookshare.db`
if test -z "$perm"; then
	echo -n "-1"
else
	echo -n "$perm"
fi
