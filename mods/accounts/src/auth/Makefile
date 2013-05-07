all: test_login test_auth

test_login: login.c
	cc -o test_login -s login.c -lcrypt -lsqlite3

test_auth: auth.c
	cc -o test_auth auth.c -lsqlite3

push: all
	cp test_login /home/ghandi/web/cgi-bin/login
	cp test_auth /home/ghandi/mods/accounts/auth
