/*
	auth
	Written by Mike Perron (2012)

	Checks cookied username and session-ID against the one stored on the
	server.

	Success is reported by returning "OK username" on stdout.
	Failure is reported by returning "NO reason" on stdout.
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
char *result_user=NULL;
int result_perm=-1;

//Local functions
void no_auth(const char *msg);
void give_auth(const char *user);
void close_tag();
static int sid_callback(void *NotUsed, int argc, char **argv, char **azColName);
static int perm_callback(void *NotUsed, int argc, char **argv, char **azColName);


/* Start of Main Function */
int main(){
	char *cookie,*ip,*user,*sid;
	char *str,*s,*a;
	FILE *infile;
	char auth=0;
	int c,i;

	sqlite3 *db;
	fprintf(stderr, "\t-- accounts:auth running --\n");
	atexit(close_tag);

	//Get environment variables.
	cookie=getenv("HTTP_COOKIE");
	ip=getenv("REMOTE_ADDR");
	if(!cookie||!ip){
		fprintf(stderr, "IP was %s; Cookie was %s\n",ip,cookie);
		no_auth("No cookie or ip.");
	}

	//Parse sid
	sid=calloc(128,sizeof(char));
	*sid=0;
	a=str;
	if(a=strstr(cookie,"sid=")){
		for(s=a;*s;s++)
			if(*s==';'||*s=='&')
				break;
		*s=0;
		strcpy(sid,a+4);
	}

	//Find SID in the database to get user name.
	str=calloc(256,sizeof(char));
	sprintf(str,"select user from sids where sid=\"%s\";",sid);
	if(sqlite3_open(knet_sids_location,&db)){
		sqlite3_close(db);
		return 0;
	}
	a=NULL;
	switch(sqlite3_exec(db, str, sid_callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_free(a);
			sqlite3_close(db);
	}
	sqlite3_close(db);
	fprintf(stderr,"User was %s.\n",result_user);
	if(!result_user)
		auth|=1;
	else user=result_user;

	//Check permissions of the user.
	sprintf(str,"select permission_level from users where username=\"%s\";",result_user);
	if(sqlite3_open(knet_db_location,&db)){
		sqlite3_close(db);
		return 0;
	}
	a=NULL;
	switch(sqlite3_exec(db, str, perm_callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_free(a);
			sqlite3_close(db);
	}
	sqlite3_close(db);
	fprintf(stderr,"Permission level was %d.\n",result_perm);
	if(result_perm<0)
		auth|=2;

	//If both checks pass, we're in good shape and the user can be authorized.
	if(!auth)give_auth(user); else {
		switch(auth){
			case 1:
				fprintf(stderr,"SID looked bad.\n");
				no_auth("Bad SID");
				break;
			case 2:
				fprintf(stderr,"Account seems locked.\n");
				no_auth("Locked account");
				break;
			default:
				fprintf(stderr,"Issues abound. (0x1)\n");
				no_auth("Bad SID or locked account");
		}
	}
	return 0;
}
/* End of Main Function */

void no_auth(const char *msg){
	printf("NO %s",msg);
	exit(0);
}
void give_auth(const char *user){
	printf("OK %s",user);
	exit(0);
}

void close_tag(){
	fprintf(stderr,"\t-- accounts:auth done --\n");
}

static int sid_callback(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		if(!strcmp(*azColName,"user"))
			if(*argv)
				strcpy(result_user=calloc(1+strlen(*argv),sizeof(char)),*argv);
		argv++,azColName++;
	}
	return 0;
}

static int perm_callback(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		if(!strcmp(*azColName,"permission_level"))
			result_perm=atoi(*argv);
		argv++,azColName++;
	}
	return 0;
}
