#include "pxutil.h"

char *gettok( char *str , const char *delim )
{
	static char	*copy = NULL ;
	static char	*save = NULL ;
	char		*p = NULL ;
	
	if( str )
	{
		if( copy )
		{
			free( copy );
			copy = NULL ;
			save = NULL ;
		}
		
		copy = strdup( str ) ;
		if( copy == NULL )
			return NULL;
		
		p = strtok_r( copy , delim , & save ) ;
		if( p == NULL )
		{
			free( copy );
			copy = NULL ;
			save = NULL ;
		}
		
		return p;
	}
	else
	{
		p = strtok_r( NULL , delim , & save ) ;
		if( p == NULL )
		{
			if( copy )
			{
				free( copy );
				copy = NULL ;
				save = NULL ;
			}
		}
		
		return p;
	}
}

char *TrimEnter( char *str )
{
	char	*ptr = NULL ;
	
	if( str == NULL )
		return NULL;
	
	for( ptr = str + strlen(str) - 1 ; ptr >= str ; ptr-- )
	{
		if( (*ptr) == '\r' || (*ptr) == '\n' )
			(*ptr) = '\0' ;
		else
			break;
	}
	
	return str;
}

