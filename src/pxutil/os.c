#include "pxutil.h"

char *GetUsernamePtr()
{
	struct passwd	*pwd = NULL ;
	
	pwd = getpwuid( getuid() ) ;
	if( pwd == NULL )
		return NULL;
	
	return pwd->pw_name;
}

