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
	if(!str)e++; else {
		if(!strncmp(str,"NO",2))e++;
		if(strncmp(str,"OK",2))e++;
	}
	user=getenv("kraknet_user");if(!user)e++;
	ip=getenv("kraknet_user_ip");if(!ip)e++;

	if(!e)printf("<span style=\"white-space:pre;\">Hello, %s!\n(%s)\n<a href=/manager/>Manage</a>&nbsp;|&nbsp;<a href=/logout.cgi>Logout</a></span>",user,ip);
	else printf("<a href=/login.cgi>Login</a>");
	return 0;
}
