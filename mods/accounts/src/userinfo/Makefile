all: user_info

user_info: main.c
	cc -o user_info main.c

push: all
	cp user_info /home/ghandi/mods/accounts/

clean:
	rm user_info
