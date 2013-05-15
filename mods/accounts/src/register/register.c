/*
	register
	based on add_user
	Written by Mike Perron (2013)

	Crudely hacked together tool to add users to Kraknet.
	Crudely modified to allow users to add themselves.
	Crudely retrofitted with SQLite3.
	Crudely adapted to bookshare on krakws.

	TODO: Must have email and permission_level set to create a user in this
	database.
*/
#define _XOPEN_SOURCE

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <regex.h>
#include <sqlite3.h>

#define knet_db_location "/home/ghandi/db/bookshare.db"

char *result_user=NULL;
enum post_mode { pm_null, pm_user, pm_pw, pm_pwr, pm_email };

char x2c(char *what);
int sanitize(char *a);
void unescape_url(char *url);
static int callback_user(void *NotUsed, int argc, char **argv, char **azColName);
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

static void fail(const char *msg);

/* Start of main function */
int main(int argc, char**argv){
	char *user,*pass,*passr,*salt,*cpt,*hash;
	enum post_mode mode=pm_null;
	char *str,*p,*s,*a;
	int post_length;
	sqlite3 *db;
	size_t n;

	// Email hackery
	char *email=calloc(256,sizeof(char));
	*email=0;
	char *es;

	regex_t loweral,min4,fsuedu;
	regcomp(&loweral,"\\`[a-z]\\{4,31\\}[0-9]\\{0,2\\}\\'",REG_NOSUB);
	regcomp(&min4,"\\`.\\{4,63\\}\\'",REG_NOSUB);
	regcomp(&fsuedu,"fitchburgstate\\.edu\\'",REG_NOSUB);

	//Get POST info
	str=calloc(n=256,sizeof(char));
	getline(&str,&n,stdin);
	post_length=sanitize(str);


	//Get user and password from POST.
	passr=calloc(64,sizeof(char));*passr=0;
	pass=calloc(64,sizeof(char));*pass=0;
	user=calloc(32,sizeof(char));*user=0;
	for(a=s=str;s-str<post_length;s++){
		if(*s=='='){
			*(s++)=0;
			if(!strcmp(a,"user"))
				mode=pm_user;
			else if(!strcmp(a,"email"))
				mode=pm_email;
			else if(!strcmp(a,"pw"))
				mode=pm_pw;
			else if(!strcmp(a,"pwr"))
				mode=pm_pwr;
			else mode=pm_null;
			for(a=s;*s;s++)
				if(*s==';'||*s=='&')
					break;
			*s=0;
			switch(mode){
				case pm_user:
					strcpy(user,a);
					break;
				case pm_pw:
					strcpy(pass,a);
					unescape_url(pass);
					break;
				case pm_pwr:
					strcpy(passr,a);
					unescape_url(passr);
					break;
				case pm_email:
					strcpy(email,a);
					unescape_url(email);
					break;
			}
			a=s+1;
		}
	}
	if(*email){
		if(regexec(&fsuedu,email,0,NULL,0))
			fail("Must be a fitchburgstate.edu address.");
		strcpy(user,email);
		if(es=strchr(user,'@'))
			*es=0;
		else fail("Not an email address.");
	}
	if(!*user||!*pass||!*passr)
		fail("Missing information.");
	if(strcmp(pass,passr))
		fail("Passwords do not match.");
	if(regexec(&min4,pass,0,NULL,0))
		fail("Password must be at least 4 characters and no more than 63.");
	if(regexec(&loweral,user,0,NULL,0))
		fail("Username must be 4 to 31 characters, and all lowercase letters.");

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
	sprintf(str,"select username from users where username=\"%s\";",user);
	if(sqlite3_open(knet_db_location,&db))
		sqlite3_close(db);
	a=NULL;
	switch(sqlite3_exec(db, str, callback_user, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_close(db);
			fail("Database error.");
	}
	if(result_user){
		sqlite3_close(db);
		fail("User name already in use.");
	}
	sqlite3_free(a);
	sqlite3_close(db);

	//If not, add user.
	sprintf(str,"insert into users(username,password_hash,email,permission_level) values(\"%s\",\"%s\",\"%s\",\"1\");",user,hash,email);
	if(sqlite3_open(knet_db_location,&db))
		sqlite3_close(db);
	a=NULL;
	switch(sqlite3_exec(db, str, callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_close(db);
			fail("Database error.");
	}
	sqlite3_free(a);
	sqlite3_close(db);

	// Begin the reply.
	printf(
		"Content-type: text/html; charset=UTF-8\n\n"
		"<!DOCTYPE html>\n<html><head>\n"
		"<title>Kraknet Registration</title>\n"
		"<meta http-equiv=\"refresh\" content=\"1;url=/register.html\">\n"
		"</head><body>\n"
		"Registration successful. <a href=/register.html>Login</a> with your new credentials or wait to be redirected.\n"
		"</body></html>\n"
	);
	return 0;
}
/* End of main function */

int sanitize(char *a){
	int l=0;
	do if(*a=='\n'||*a=='\r')
		break;
	while(*(a++)&&++l);
	*a=0;
	return l;
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
		if(!strcmp(*azColName,"username"))
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

static void fail(const char *msg){
	printf(
		"Content-type: text/html; charset=UTF-8\n\n"
		"<!DOCTYPE html>\n<html><head>\n"
		"<title>(Failed) Kraknet Registration</title>\n"
		"<meta http-equiv=\"refresh\" content=\"2;url=/register.html\">\n"
		"</head><body>\n"
		"%s<br>\n"
		"You will be redirected to the registration page.<br>\n"
		"</body></html>\n",
		msg
	);
	exit(-1);
}
