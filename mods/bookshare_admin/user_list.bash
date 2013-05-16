#!/bin/bash
# Generates a table row for each user in the database.
# The assumption is that a table is already on the page.
# Written by Mike Perron (2013)

request="select username,permission_level,date_joined,email from users;"

echo ""
IFS=$'\n'
for USERLINE in $(echo "$request" | sqlite3 /home/ghandi/db/bookshare.db); do
	echo "<tr>"
	IFS=$'|'
	i=0
	for FIELD in $USERLINE; do
		case $i in
			0) 
				echo -e "\t<td><input type=checkbox name=\"user_$FIELD\"></td>"
				i=1
			;&
			*) echo -e "\t<td>$FIELD</td>" ;;
		esac
	done
	echo "</tr>"
	IFS=$'\n'
done
