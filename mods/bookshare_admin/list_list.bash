#!/bin/bash
# Creates an HTML option tag for each listing in the Bookshare database.
# The assumption is that a select tag is already on the page.
# Written by Mike Perron (2013)

request="select books.title,books.isbn,listings.date_posted,users.username,listings.book_ref_id,listings.user_id from listings inner join books on listings.book_ref_id=books.book_ref_id inner join users on listings.user_id=users.user_id order by books.title;"

IFS=$'\n'
for LISTLINE in $(echo "$request" | sqlite3 /home/ghandi/db/bookshare.db); do
	IFS="|"
	i=0
	for VALUE in $LISTLINE; do
		case "$i" in
			0) TITLE=$VALUE ;;
			1) ISBN=$VALUE ;;
			2) DATE_P=$VALUE ;;
			3) USERNAME=$VALUE ;;
			4) BOOK_ID=$VALUE ;;
			5) USER_ID=$VALUE ;;
		esac
		i=$(expr $i + 1)
	done
	TITLE_S=$(expr substr "$TITLE" 1 20)
	if [ ${#TITLE} -ne ${#TITLE_S} ]; then
		TITLE="$TITLE_S..."
	else
		TITLE=$TITLE_S
	fi
	REF_ID="${USER_ID}_${BOOK_ID}"
	echo "<option value=$REF_ID>$ISBN: $TITLE ($USERNAME on $DATE_P)</option>"
	IFS=$'\n'
done
