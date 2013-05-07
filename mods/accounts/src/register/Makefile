all: add_user

add_user: register.c
	cc -o add_user -s register.c -lcrypt -lsqlite3

push: all
	cp add_user /home/ghandi/web/cgi-bin/register
