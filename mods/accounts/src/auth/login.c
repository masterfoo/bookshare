/*
	login
	Written by Mike Perron (2013)

	CGI for logins to Kraknet. This should compute the hash of the user's
	password, and then interface with SQLite to compare the stored hash. If
	everything checks out (the user exists and the hashes match) build a
	session ID from 32 bytes of /dev/urandom data, store that to a SQLite
	database on tmpfs, and return it as a cookie to the user.
*/
#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

#define knet_db_location "/home/ghandi/db/bookshare.db"
#define knet_sids_location "/home/ghandi/db/sids.db"

//Result of SQL requests
char *result_hash=NULL;

//Local functions
char x2c(char *what);
void unescape_url(char *url);
char hex15(int val);
void fail(const char *msg);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

/* Start of main function */
int main(){
	char *str,*s,*a;
	sqlite3 *db;
	FILE *urnd;
	size_t n;
	int i,c;

	int persistent=0;
	char *user,*pass,*ref,*sid;
	char salt[3],*hash,chunk[9];

	//Opening headers
	printf("Content-Type: text/html; charset=UTF-8\n");

	//Read POST (fail on empty)
	str=calloc(n=256,sizeof(char));
	if(getline(&str,&n,stdin)<=0)
		fail("No POST data. (0x7)");
	for(s=str;*s;s++)
		if(*s=='\r'||*s=='\n')
			break;
	*s=0;

	//Parse user, pass, and ref from POST
	user=calloc(32,sizeof(char));
	pass=calloc(64,sizeof(char));
	ref=calloc(n,sizeof(char));
	*user=*pass=*ref=0;

	a=str;
	do{	for(s=a;*s;s++)
			if(*s=='&'||*s==';')
				break;
		i=*s;
		*s=0;
		if(!strncmp(a,"user=",5))
			strcpy(user,a+5);
		else if(!strncmp(a,"pass=",5))
			strcpy(pass,a+5);
		else if(!strncmp(a,"ref=",4))
			strcpy(ref,a+4);
		else if(!strcmp(a,"persist=on"))
			persistent=1;

		*s=i;
		a=s+1;
	}	while(i);

	unescape_url(user);
	unescape_url(pass);
	unescape_url(ref);


	//Find user in the database and read in their password hash
	sprintf(str,"select users.password_hash from users where users.username=\"%s\";",user);
	if(sqlite3_open(knet_db_location,&db)){
		sqlite3_close(db);
		fail("Could not attach to database (0x1)");
	}

	a=NULL;
	switch(sqlite3_exec(db, str, callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_free(a);
			sqlite3_close(db);
			fail("Hash recovery error. (0x6)");
	}
	sqlite3_close(db);
	if(!result_hash)
		fail("Hash not present. (0x5)");

	//Hash pass from POST
	memset(chunk,0,9*sizeof(char));
	memset(salt,0,3*sizeof(char));
	strncpy(salt,(a=pass)+2,2);
	*(hash=calloc(256,sizeof(char)))=0;
	while(1){
		if(strlen(a)>8){
			memset(chunk,0,9*sizeof(char));
			strncpy(chunk,a,8);

			s=crypt(chunk,salt)+2;
			strcat(hash,s);
			a+=8;
		} else {
			s=crypt(a,salt)+2;
			strcat(hash,s);
			break;
		}
	}
	fprintf(stderr,"computed hash: %s\n",hash);

	//Compare passwords
	if(strcmp(hash,result_hash))
		fail("Password mismatch. (0x4)");

	//Get a session ID from random data
	urnd=fopen("/dev/urandom","r");
	sid=calloc(256,sizeof(char));
	memset(sid,0,256);
	if(!urnd)
		return -1;
	for(i=32,s=sid;i--;s++){
		c=getc(urnd);
		*(s++)=hex15((c>>4)&15);
		*s=hex15(c&15);
	}
	fclose(urnd);

	//Set the SID cookie (NOTE: user name is no longer cookied.)
	printf("Set-Cookie: sid=%s; Path=/%s\n",sid,(persistent)?"; Max-Age=2620800":"");

	//Save SID to SQLite DB
	sprintf(str,"insert into sids(sid,user,ip) values (\"%s\",\"%s\",\"%s\");",sid,user,getenv("REMOTE_ADDR"));
	if(sqlite3_open(knet_sids_location,&db)){
		sqlite3_close(db);
		fail("Insertion error. (0x3)");
	}

	a=NULL;
	switch(sqlite3_exec(db, str, callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_free(a);
			sqlite3_close(db);
			fail("SQLite Error. (0x2)");
	}
	sqlite3_close(db);

	//Redirect
	if(strstr(ref,"/register.html")||!*ref)
		strcpy(ref,"/");
	if(s=getenv("HTTP_COOKIE"))
		if(strstr(s,"isbn="))
			strcpy(ref,"/share.cgi");
	printf(
		"\n<!DOCTYPE html>\n"
		"<html><head><meta http-equiv=\"refresh\" content=\"1;url=%s\">Redirecting to %s ...</head></html>\n",
		ref,ref
	);
	return 0;
}
/* End of main function */

char x2c(char *what){
	register char digit;
	digit=(what[0]>='A'?((what[0]&0xdf)-'A')+10:(what[0]-'0'));
	digit*=16;
	digit+=(what[1]>='A'?((what[1]&0xdf)-'A')+10:(what[1]-'0'));
	return digit;
}
void unescape_url(char *url){
	register int x,y;
	for(x=0,y=0;url[y];++x,++y){
		if((url[x]=url[y])=='%'){
			url[x]=x2c(&url[y+1]);
			y+=2;
		}
	}
	url[x]=0;
}
char hex15(int val){
	if(val<10)
		return '0'+val;
	return val+'a'-10;
}
void fail(const char *msg){
	printf(
		"\n<!DOCTYPE html>\n"
		"<html><head><meta http-equiv=\"refresh\" content=\"1;url=/register.html?m=f\"></head><body>Failed.<br>%s</body></html>\n",
		msg
	);
	exit(0);
}

/* SQL Callback Function */
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		if(!strcmp(*azColName,"password_hash"))
			strcpy(result_hash=calloc(1+strlen(*argv),sizeof(char)),*argv);
		argv++,azColName++;
	}
	return 0;
}
