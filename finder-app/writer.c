#include "stdio.h"
#include "syslog.h"
#include <string.h>
#include <sys/syslog.h>



int main (int argc, char *argv[]) {
	
	openlog(NULL,LOG_PERROR,LOG_USER);
	if (argc != 3){
		syslog(LOG_ERR,"number of arguments is not correct");	
		return 1;
	}

	syslog(LOG_DEBUG,"writing %s to %s", argv[2],argv[1]);

	FILE * file = fopen(argv[1],"w");

	if (!file){
		syslog(LOG_DEBUG, "file not found");
		return 1;
	}


	size_t ret = fwrite(argv[2],1,strlen(argv[2]),file);
	printf("chars written are %ld",ret);
	if (ret != strlen(argv[2])){
		syslog(LOG_ERR, "FILE WRITE FAILED");
		fclose(file);
		return 1;
	}

	fclose(file);

}



