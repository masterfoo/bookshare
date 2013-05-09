#!/usr/bin/python
# Adds a referrer to the page if appropriate.
import sys
import re
import os

refex=re.compile(r".*/cgi-bin/.*$")
if 'HTTP_REFERER' in os.environ:
	ref=os.environ['HTTP_REFERER']
	sys.stdout.write("<input type=hidden name=ref value=\"")
	if refex.match(ref):
		sys.stdout.write("/")
	else:
		sys.stdout.write(ref)
	sys.stdout.write("\">")
else:
	sys.stdout.write("<input type=hidden name=ref value=\"/\">")
