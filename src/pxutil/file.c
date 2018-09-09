#include "pxutil.h"

int PXReadEntireFile( char *pathfilename , char **pp_file_buffer , int *p_file_size )
{
	FILE		*fp = NULL ;
	int		file_size ;
	char		*file_buffer = NULL ;
	
	int		nret = 0 ;
	
	fp = fopen( pathfilename , "r" ) ;
	if( fp == NULL )
	{
		return -1;
	}
	
	fseek( fp , 0 , SEEK_END );
	file_size = ftell( fp ) ;
	
	fseek( fp , 0 , SEEK_SET );
	file_buffer = (char *)malloc( file_size+1 ) ;
	if( file_buffer == NULL )
	{
		fclose( fp );
		return -2;
	}
	
	nret = fread( file_buffer , file_size , 1 , fp ) ;
	if( nret != 1 )
	{
		fclose( fp );
		return -3;
	}
	file_buffer[file_size] = '\0' ;
	
	fclose( fp );
	
	if( pp_file_buffer )
		(*pp_file_buffer) = file_buffer ;
	if( p_file_size )
		(*p_file_size) = file_size ;
	return 0;
}

