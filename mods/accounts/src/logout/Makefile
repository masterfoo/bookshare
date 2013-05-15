all: test_logout

test_logout: logout.c
	cc -o test_logout -s logout.c -lsqlite3

push: all
	cp test_logout /home/ghandi/web/cgi-bin/logout
