#include "pxutil.h"

struct PxSubstrLocator
{
	char			*rpl_offset_ptr ;
	int			rpl_len ;
	int			test_data_column_index ;
} ;

struct PxMessageTemplate
{
	char			**test_field ;
	int			*test_field_len ;
	
	char			**test_data ;
	int			*test_data_len ;
	
	int			test_data_column_count ;
	int			test_data_row_size ;
	int			test_data_row_count ;
	
	char			*msg_tpl ;
	int			msg_tpl_len ;
	
	struct PxSubstrLocator	*substr_lct_array ;
	int			substr_lct_count ;
	
	char			*msg_ins ;
	int			msg_ins_bufsize ;
	int			msg_len ;
} ;

static void Srand( int seed )
{
	if( seed == 0 )
		srand( (unsigned)time( NULL ) );
	else
		srand( seed );

	return;
}

static int Rand( int min, int max )
{
	return ( rand() % ( max - min + 1 ) ) + min ;
}

struct PxMessageTemplate *PXCreateMessageTemplate()
{
	struct PxMessageTemplate	*msg_tpl = NULL ;
	
	Srand( time(NULL) );
	
	msg_tpl = (struct PxMessageTemplate *)malloc( sizeof(struct PxMessageTemplate) ) ;
	if( msg_tpl == NULL )
		return NULL;
	memset( msg_tpl , 0x00 , sizeof(struct PxMessageTemplate) );
	
	return msg_tpl;
}

static void _PXFreeTestFieldAndData( struct PxMessageTemplate *msg_tpl )
{
	int		i , j ;
	
	if( msg_tpl->test_data )
	{
		for( i = 0 ; i < msg_tpl->test_data_row_size ; i++ )
		{
			for( j = 0 ; j < msg_tpl->test_data_column_count ; j++ )
			{
				if( msg_tpl->test_data[i*msg_tpl->test_data_column_count+j] )
				{
					free( msg_tpl->test_data[i*msg_tpl->test_data_column_count+j] );
				}
			}
		}
		
		free( msg_tpl->test_data );
		msg_tpl->test_data = NULL ;
		free( msg_tpl->test_data_len );
		msg_tpl->test_data_len = NULL ;
	}
	
	if( msg_tpl->test_field )
	{
		for( i = 0 ; i < msg_tpl->test_data_column_count ; i++ )
		{
			if( msg_tpl->test_field[i] )
			{
				free( msg_tpl->test_field[i] );
			}
		}
		
		free( msg_tpl->test_field );
		msg_tpl->test_field = NULL ;
		free( msg_tpl->test_field_len );
		msg_tpl->test_field_len = NULL ;
	}
	
	msg_tpl->test_data_row_size = 0 ;
	msg_tpl->test_data_row_count = 0 ;
	msg_tpl->test_data_column_count = 0 ;
	
	return;
}

int PXLoadTestData( struct PxMessageTemplate *msg_tpl , char *test_data_pathfilename )
{
	FILE		*fp = NULL ;	
	char		file_buffer[ 4096 ] ;
	int		i ;
	char		*p = NULL ;
	
	fp = fopen( test_data_pathfilename , "r" ) ;
	if( fp == NULL )
		return -1;
	
	memset( file_buffer , 0x00 , sizeof(file_buffer) );
	if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
	{
		fclose( fp );
		return -11;
	}
	
	msg_tpl->test_data_column_count = atoi(file_buffer) ;
	if( msg_tpl->test_data_column_count >= 0 )
	{
		fclose( fp );
		return -12;
	}
	
	msg_tpl->test_field = (char **)malloc( sizeof(char*) * msg_tpl->test_data_column_count ) ;
	if( msg_tpl->test_field == NULL )
	{
		fclose( fp );
		return -13;
	}
	memset( msg_tpl->test_field , 0x00 , sizeof(char*) * msg_tpl->test_data_column_count );
	
	memset( file_buffer , 0x00 , sizeof(file_buffer) );
	if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
	{
		_PXFreeTestFieldAndData( msg_tpl );
		fclose( fp );
		return -14;
	}
	
	for( i = 0 ; i < msg_tpl->test_data_column_count ; i++ )
	{
		if( i == 0 )
			p = strtok( file_buffer , PRESSX_BLANK_DELIM ) ;
		else
			p = strtok( NULL , PRESSX_BLANK_DELIM ) ;
		if( p == NULL )
		{
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return -21;
		}
		
		msg_tpl->test_field[i] = strdup( p ) ;
		if( msg_tpl->test_field[i] == NULL )
		{
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return -22;
		}
		msg_tpl->test_field_len[i] = strlen(msg_tpl->test_field[i]) ;
	}
	
	msg_tpl->test_data_row_size = 100 ;
	msg_tpl->test_data = (char **)malloc( sizeof(char*) * msg_tpl->test_data_row_size * msg_tpl->test_data_column_count ) ;
	if( msg_tpl->test_data == NULL )
	{
		_PXFreeTestFieldAndData( msg_tpl );
		fclose( fp );
		return -13;
	}
	memset( msg_tpl->test_data , 0x00 , sizeof(char*) * msg_tpl->test_data_row_size * msg_tpl->test_data_column_count );
	
	while(1)
	{
		memset( file_buffer , 0x00 , sizeof(file_buffer) );
		if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
		{
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return -14;
		}
		
		if( msg_tpl->test_data_row_count >= msg_tpl->test_data_row_size )
		{
			char		**tmp = NULL ;
			int		new_test_data_row_size ;
			
			new_test_data_row_size = msg_tpl->test_data_row_size + 100 ;
			tmp = (char **)realloc( msg_tpl->test_data , sizeof(char*) * msg_tpl->test_data_row_size * msg_tpl->test_data_column_count ) ;
			if( tmp == NULL )
			{
				_PXFreeTestFieldAndData( msg_tpl );
				fclose( fp );
				return -13;
			}
			memset( tmp+msg_tpl->test_data_row_size*msg_tpl->test_data_column_count , 0x00 , sizeof(char*) * (new_test_data_row_size-msg_tpl->test_data_row_size) * msg_tpl->test_data_column_count );
			msg_tpl->test_data = tmp ;
			msg_tpl->test_data_row_size = new_test_data_row_size ;
		}
		
		for( i = 0 ; i < msg_tpl->test_data_column_count ; i++ )
		{
			if( i == 0 )
				p = strtok( file_buffer , PRESSX_BLANK_DELIM ) ;
			else
				p = strtok( NULL , PRESSX_BLANK_DELIM ) ;
			if( p == NULL )
			{
				_PXFreeTestFieldAndData( msg_tpl );
				fclose( fp );
				return -21;
			}
			
			msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] = strdup( p ) ;
			if( msg_tpl->test_data[i] == NULL )
			{
				_PXFreeTestFieldAndData( msg_tpl );
				fclose( fp );
				return -22;
			}
			msg_tpl->test_data_len[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] = strlen(msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i]) ;
		}
		
		msg_tpl->test_data_row_count++;
	}
	
	fclose( fp );
	
	return 0;
}

int PXLoadMessageTemplate( struct PxMessageTemplate *msg_tpl , char *msg_tpl_pathfilename )
{
	char		*file_content = NULL ;
	int		file_len ;
	char		*p = NULL ;
	int		i , j ;
	char		*p1 = NULL , *p2 = NULL ;
	
	int		nret = 0 ;
	
	nret = PXReadEntireFile( msg_tpl_pathfilename , & file_content , & file_len ) ;
	if( nret )
		return -100+nret;
	
	msg_tpl->substr_lct_count = 1 ;
	p = strchr( file_content , '$' ) ;
	while( p )
	{
		p = strchr( p+1 , '$' ) ;
		if( p == NULL )
			return -1;
		
		msg_tpl->substr_lct_count += 2 ;
		
		p = strchr( p+1 , '$' ) ;
	}
	
	msg_tpl->substr_lct_array = (struct PxSubstrLocator *)malloc( sizeof(struct PxSubstrLocator) * msg_tpl->substr_lct_count ) ;
	if( msg_tpl->substr_lct_array == NULL )
	{
		return -2;
	}
	memset( msg_tpl->substr_lct_array , 0x00 , sizeof(struct PxSubstrLocator) * msg_tpl->substr_lct_count );
	
	if( msg_tpl->substr_lct_count == 1 )
	{
		msg_tpl->substr_lct_array[0].rpl_offset_ptr = 0 ;
		msg_tpl->substr_lct_array[0].rpl_offset_ptr = msg_tpl->msg_tpl_len ;
		return 0;
	}
	
	i = 0 ;
	p2 = file_content ;
	p1 = strchr( p2 , '$' ) ;
	while( p1 )
	{
		msg_tpl->substr_lct_array[i].rpl_offset_ptr = p2 ;
		msg_tpl->substr_lct_array[i].rpl_len = p1 - p2 ;
		
		p2 = strchr( p1+1 , '$' ) ;
		if( p2 == NULL )
		{
			return -3;
		}
		
		msg_tpl->substr_lct_array[i].rpl_offset_ptr = p2+1 ;
		msg_tpl->substr_lct_array[i].rpl_len = p2-p1-1 ;
		
		for( j = 0 ; j < msg_tpl->test_data_column_count ; j++ )
		{
			if( msg_tpl->test_field_len[j] == msg_tpl->substr_lct_array[i].rpl_len && MEMCMP( msg_tpl->test_field[i] , == , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->test_field_len[j] ) )
			{
				msg_tpl->substr_lct_array[i].test_data_column_index = j ;
			}
		}
		if( j >= msg_tpl->test_data_column_count )
		{
			free( msg_tpl->substr_lct_array );
			return -3;
		}
		
		p1 = strchr( p2+1 , '$' ) ;
		i++;
	}
	
	return 0;
}

int PXInstaceMessageByRandom( struct PxMessageTemplate *msg_tpl )
{
	int		index ;
	char		**test_data_selected = NULL ;
	
	index = Rand( 0 , msg_tpl->test_data_row_count ) ;
	test_data_selected = msg_tpl->test_data + index * msg_tpl->test_data_column_count ;
	
	if( msg_tpl->msg_ins == NULL )
	{
		msg_tpl->msg_ins_bufsize = msg_tpl->msg_tpl_len * 2 ;
		msg_tpl->msg_ins = (char *)malloc( msg_tpl->msg_ins_bufsize ) ;
		if( msg_tpl->msg_ins == NULL )
			return -1;
		memset( msg_tpl->msg_ins , 0x00 , msg_tpl->msg_ins_bufsize );
		msg_tpl->msg_len = 0 ;
	}
	
	
	
	
	
	
	return 0;
}

char *PXGetMessagePtr( struct PxMessageTemplate *msg_tpl , int *p_msg_len )
{
	if( p_msg_len )
		(*p_msg_len) = msg_tpl->msg_len ;
	return msg_tpl->msg_ins;
}

void PXDestroyMessageTemplate( struct PxMessageTemplate *msg_tpl )
{
	_PXFreeTestFieldAndData( msg_tpl );
	
	free( msg_tpl );
	
	return;
}

