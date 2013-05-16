/*
	operate
	Written by Mike Perron (2013)

	Performs SQL operations as a helper for the /admin/ UI.
	sudo: adjust permission_level of a set of users.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int chop(char *a);
static char **parse_t(char *a);

int main(){
	char op[5],code[256];
	char *a,*s,*str;
	size_t n;
	int i;

	char **user_t=NULL,**cp_it;
	FILE *pipe;

	// Crude strap to support POST.
	str=calloc(n=256,sizeof(char));
	getline(&str,&n,stdin);
	setenv("QUERY_STRING",str,1);

	// Check user permission. Must be 0.
	if(!(pipe=popen("mod_find accounts:permissions","r"))){
		printf("Unauthorized. Actions disabled.<br>\n");
		return 0;
	}
	getline(&str,&n,pipe);
	i=atoi(str);
	if(i!=*str-0x30){
		printf("Unauthorized. Actions disabled.<br>\n");
		return 0;
	}
	pclose(pipe);
	free(str);

	if(!(a=getenv("QUERY_STRING")))
		return 0;

	// Preserve QUERY_STRING.
	str=calloc(n=strlen(a)+1,sizeof(char));
	strcpy(str,a);

	// Get operating mode.
	if(!(a=strstr(str,"op=")))
		return 0;
	chop(a);
	strncpy(op,a+3,4);
	4[op]=0;

	// Get a code if there is one.
	strcpy(str,getenv("QUERY_STRING"));
	if(a=strstr(str,"code=")){
		chop(a);
		strcpy(code,a+5);
	} else 0[code]=0;

	// Get targets.
	strcpy(str,getenv("QUERY_STRING"));
	user_t=parse_t(str);

	// Open a pipe to sqlite3.
	strcpy(str,"sqlite3 /home/ghandi/db/bookshare.db");
	if(!(pipe=popen(str,"w"))){
		printf("Can't write database.<br>\n");
		return -1;
	}

	/* Adjust permission levels with sudo */
	if(!strcmp(op,"sudo")){
		if(!code){
			printf("Operation requires a code parameter.<br>\n");
			return -1;
		}
		i=atoi(code);
		for(cp_it=user_t;*cp_it;cp_it++)
			fprintf(pipe,"update users set permission_level=\"%d\" where username=\"%s\";",i,*cp_it);
	}

	// Clean up.
	pclose(pipe);
	free(str);
	return 0;
}


/* Cut a phrase from a &/; deliniated string. */
static int chop(char *a){
	int l=0;
	if(!*a)
		return 0;

	do if(*a==';'||*a=='&')
		break;
	while(l++, *(++a));
	*a=0;

	return l;
}

/* Grab users from a list that looks like user_username=on */
static char **parse_t(char *a){
	char **list=NULL;
	char *p,*s,*b;
	int c=0;

	b=calloc(1+strlen(a),sizeof(char));

	while(1){
		list=realloc(list,++c*sizeof(char*));
		*(list+c-1)=NULL;

		if(!(s=strstr(a,"user_")))
			break;
		strcpy(b,a=s+5);
		chop(b);

		if(!(s=strstr(b,"=on")))
			continue;
		*s=0;

		p=*(list+c-1)=calloc(1+strlen(b),sizeof(char));
		strcpy(p,b);
	}

	free(b);
	return list;
}
