#!/bin/sh

echo "Content-Type: text/html
Set-Cookie: sid=deleted; Expires=Thu, 01 Jan 1970 00:00:01 GMT; Path=/; Domain=.krakissi.net

<!DOCTYPE html>
<html><head>
	<title>Killing Your Session</title>
	<meta http-equiv=\"refresh\" content=\"0;url=$HTTP_REFERER\">
</head>
</html>"

if [ $QUERY_STRING ]; then
	echo "delete from sids where sid=\""$QUERY_STRING"\";" | sqlite3 /tmp/kraknet/sids.db
fi
