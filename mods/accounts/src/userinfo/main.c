/*
	user_info
	Written by Mike Perron (2012)

	This program simply checks environment variables Kraknet will have set if
	the user is authenticated, and greets them. If the variables aren't set, or
	if $kraknet_user_auth is NO, or not OK, a link to login is displayed
	instead.
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char**argv){
	char *user,*ip;
	char *str;
	int e=0;

	str=getenv("kraknet_user_auth");
	if(!str)
		e++;
	else {
		if(!strncmp(str,"NO",2))
			e++;
		if(strncmp(str,"OK",2))
			e++;
	}
	if(!(user=getenv("kraknet_user")))
		e++;
	if(!(ip=getenv("kraknet_user_ip")))
		e++;

	if(!e)
		printf(
			"Hello, <a href=/user.html>%s</a>! <a href=/cgi-bin/logout>Logout</a>",
			user
		);
	else printf("<a href=/register.html>Login</a>");
	return 0;
}
