/*
	register
	based on add_user
	Written by Mike Perron (2013)

	Crudely hacked together tool to add users to Kraknet.
	Crudely modified to allow users to add themselves.
	Crudely retrofitted with SQLite3.
*/
#define _XOPEN_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sqlite3.h>

#define knet_db_location "/home/www-data/kraknet.db"

char *result_user=NULL;

char x2c(char *what);
int sanitize(char *a);
void close_connection();
void unescape_url(char *url);
static int callback_user(void *NotUsed, int argc, char **argv, char **azColName);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);
enum post_mode { pm_null, pm_user, pm_pw, pm_pwr }; 

/* Start of main function */
int main(int argc, char**argv){
	char *user,*pass,*passr,*salt,*cpt,*hash;
	enum post_mode mode=pm_null;
	char *str,*p,*s,*a;
	int post_length;
	sqlite3 *db;
	size_t n;

	regex_t loweral,min4;
	regcomp(&loweral,"\\`[a-z]\\{4,31\\}\\'",REG_NOSUB);
	regcomp(&min4,"\\`.\\{4,63\\}\\'",REG_NOSUB);

	//A little care for the receiving browser.
	atexit(close_connection);

	//Get POST info
	str=calloc(n=256,sizeof(char));
	getline(&str,&n,stdin);
	post_length=sanitize(str);

	//Begin the reply
	printf(	//Internet standard is \r\n
		"Content-type: text/html; charset=UTF-8\r\n\r\n"
		"<!DOCTYPE html><html><head>"
		"<title>Kraknet Registration</title></head>"
		"<body>\r\n"
	);

	//Get user and password from POST.
	passr=calloc(64,sizeof(char));*passr=0;
	pass=calloc(64,sizeof(char));*pass=0;
	user=calloc(32,sizeof(char));*user=0;
	for(a=s=str;s-str<post_length;s++){
		if(*s=='='){
			*(s++)=0;
			if(!strcmp(a,"user"))mode=pm_user;
			else if(!strcmp(a,"pw"))mode=pm_pw;
			else if(!strcmp(a,"pwr"))mode=pm_pwr;
			else mode=pm_null;
			for(a=s;*s;s++)if(*s==';'||*s=='&')break;*s=0;
			switch(mode){
				case pm_user: strcpy(user,a); break;
				case pm_pw:
					strcpy(pass,a);
					unescape_url(pass);
					break;
				case pm_pwr:
					strcpy(passr,a);
					unescape_url(passr);
					break;
			}
			a=s+1;
		}
	}
	if(!*user||!*pass||!*passr){
		printf("Missing information.\r\n");
		return -1;
	}
	if(strcmp(pass,passr)){
		printf("Passwords do not match.\r\n");
		return -1;
	}
	if(regexec(&min4,pass,0,NULL,0)){
		printf("Password must be at least 4 characters and no more than 63.\r\n");
		return -1;
	}
	if(regexec(&loweral,user,0,NULL,0)){
		printf("Username must be 4 to 31 characters, and all lowercase letters.\r\n");
		return -1;
	}

	//If everything checks out, 'generate' salt.
	salt=calloc(3,sizeof(char));
	strncpy(salt,pass+2,2);*(salt+2)=0;
	cpt=calloc(9,sizeof(char));*(cpt+8)=0;
	*(hash=calloc(128,sizeof(char)))=0;
	s=pass;
	while(*s){
		if(strlen(s)>8){
			strncpy(cpt,s,8);
			s+=8;
		} else {
			strcpy(cpt,s);
			*s=0;
		}
		strcat(hash,crypt(cpt,salt)+2);
	}
	fprintf(stderr,"[des] hash: %s\n",hash);

	//Check if user exists.
	sprintf(str,"select user from users where user=\"%s\";",user);
	if(sqlite3_open(knet_db_location,&db))sqlite3_close(db);
	a=NULL;
	switch(sqlite3_exec(db, str, callback_user, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_close(db);
			printf("Database error.\r\n");
			return -1;
	}
	if(result_user){
		printf("User name already in use.\r\n");
		sqlite3_close(db);
		return -1;
	}
	sqlite3_free(a);
	sqlite3_close(db);

	//If not, add user.
	sprintf(str,"insert into users(user,hash,pretty) values(\"%s\",\"%s\",\"%s\");",user,hash,user);
	if(sqlite3_open(knet_db_location,&db))sqlite3_close(db);
	a=NULL;
	switch(sqlite3_exec(db, str, callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_close(db);
			printf("Database error.\r\n");
			return -1;
	}
	sqlite3_free(a);
	sqlite3_close(db);

	//Done.
	printf("Ok. <a href=/login.cgi>Login</a> with your new credentials.\r\n");
	return 0;
}
/* End of main function */

int sanitize(char *a){
	int l=0;
	do if(*a=='\n'||*a=='\r')break; while(*(a++) && ++l);
	*a=0;
	return l;
}
void close_connection(){
	printf("</body></html>\r\n");
}
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

/* SQL Callback Function */
static int callback_user(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		if(!strcmp(*azColName,"user"))
			strcpy(result_user=calloc(1+strlen(*argv),sizeof(char)),*argv);
		argv++,azColName++;
	}

	return 0;
}
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		argv++,azColName++;
	}

	return 0;
}
