#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>

#define knet_db_location "/home/ghandi/db/bookshare.db"
#define knet_sids_location "/home/ghandi/db/sids.db"

void fail();
static int callback(void *NotUsed, int argc, char **argv, char **azColName);

int main(){
	char *str,*s,*a;
	char *cookie;
	char *sid;
	int i;
	sqlite3 *db;

	printf("Content-Type: text/html; charset=UTF-8\n\n");
	if(!(cookie=getenv("HTTP_COOKIE")))
		fail();

	//Parse sid
	sid=calloc(128,sizeof(char));
	a=str;
	if(a=strstr(cookie,"sid=")){
		for(s=a;*s;s++)
			if(*s==';'||*s=='&')
				break;
		*s=0;
		strcpy(sid,a+4);
	}

	//Remove
	str=calloc(256,sizeof(char));
	sprintf(str,"delete from sids where sid=\"%s\";",sid);
	if(sqlite3_open(knet_sids_location,&db)){
		sqlite3_close(db);
		fail();
	}
	a=NULL;
	switch(sqlite3_exec(db, str, callback, 0, &a)){
		case SQLITE_OK:
			break;
		default:
			fprintf(stderr,"[sql] Error: %s\n",a);
			sqlite3_free(a);
			sqlite3_close(db);
			fail();
	}
	sqlite3_close(db);

	printf("<!DOCTYPE html><html><head><title>Killing Your Session</title><meta http-equiv=\"refresh\" content=\"0;url=%s\"></head></html>\n",(a=getenv("HTTP_REFERER"))?a:"/");
	return 0;
}

void fail(){
	printf("An error occured during logout. <a href=/>Return Home</a><br>");
	exit(0);
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	while(argc--){
		fprintf(stderr,"[sql] %s: %s\n",*azColName,*argv);
		argv++,azColName++;
	}
	return 0;
}
