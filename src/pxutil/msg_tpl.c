#include "pxutil.h"

#define _DEBUG				0

#define TEST_DATA_ROW_SIZE_INIT		2
#define TEST_DATA_ROW_SIZE_INCREASE	2

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
	int			msg_ins_len ;
} ;

static char	_g_message_template_error_desc[ 256 ] = "" ;	

static void _PXFreeTestFieldAndData( struct PxMessageTemplate *msg_tpl )
{
	int		i , j ;
	
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
	
	msg_tpl->test_data_row_size = 0 ;
	msg_tpl->test_data_row_count = 0 ;
	msg_tpl->test_data_column_count = 0 ;
	
	return;
}

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

struct PxMessageTemplate *PXCompileTemplate( char *test_data_pathfilename , char *msg_tpl_pathfilename )
{
	struct PxMessageTemplate	*msg_tpl = NULL ;
	
	FILE				*fp = NULL ;	
	char				file_buffer[ 4096 ] ;
	int				i , j ;
	char				*p = NULL ;
	
	char				*p1 = NULL , *p2 = NULL ;
	
	int				nret = 0 ;
	
	Srand( time(NULL) );
	
	msg_tpl = (struct PxMessageTemplate *)malloc( sizeof(struct PxMessageTemplate) ) ;
	if( msg_tpl == NULL )
		return NULL;
	memset( msg_tpl , 0x00 , sizeof(struct PxMessageTemplate) );
	
	if( test_data_pathfilename )
	{
		fp = fopen( test_data_pathfilename , "r" ) ;
		if( fp == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "fopen[%s] failed , errno[%d]" , test_data_pathfilename , errno );
			return NULL;
		}
		
		memset( file_buffer , 0x00 , sizeof(file_buffer) );
		if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "fgets[%s] for test_data_column_count failed" , test_data_pathfilename );
			fclose( fp );
			return NULL;
		}
		
		msg_tpl->test_data_column_count = atoi(file_buffer) ;
		if( msg_tpl->test_data_column_count <= 0 )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "test_data_column_count[%d] invalid" , msg_tpl->test_data_column_count );
			fclose( fp );
			return NULL;
		}
		
#if _DEBUG
		printf( "DEBUG - 111 - msg_tpl->test_data_column_count[%d]\n" , msg_tpl->test_data_column_count );
#endif
		
		msg_tpl->test_field = (char **)malloc( sizeof(char*)*msg_tpl->test_data_column_count ) ;
		if( msg_tpl->test_field == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
			fclose( fp );
			return NULL;
		}
		memset( msg_tpl->test_field , 0x00 , sizeof(char*)*msg_tpl->test_data_column_count );
		
		msg_tpl->test_field_len = (int *)malloc( sizeof(int)*msg_tpl->test_data_column_count ) ;
		if( msg_tpl->test_field_len == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
			fclose( fp );
			return NULL;
		}
		memset( msg_tpl->test_field_len , 0x00 , sizeof(int) * msg_tpl->test_data_column_count );
		
		memset( file_buffer , 0x00 , sizeof(file_buffer) );
		if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "fgets[%s] for column name list failed" , test_data_pathfilename );
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return NULL;
		}
		
		for( i = 0 ; i < msg_tpl->test_data_column_count ; i++ )
		{
			if( i == 0 )
				p = strtok( file_buffer , PRESSX_BLANK_DELIM ) ;
			else
				p = strtok( NULL , PRESSX_BLANK_DELIM ) ;
			if( p == NULL )
			{
				snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "strtok for field name failed" );
				_PXFreeTestFieldAndData( msg_tpl );
				fclose( fp );
				return NULL;
			}
			
			msg_tpl->test_field[i] = strdup( p ) ;
			if( msg_tpl->test_field[i] == NULL )
			{
				snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "strdup failed , errno[%d]" , errno );
				_PXFreeTestFieldAndData( msg_tpl );
				fclose( fp );
				return NULL;
			}
			msg_tpl->test_field_len[i] = strlen(msg_tpl->test_field[i]) ;
		}
		
#if _DEBUG
		{
			int		i ;
			for( i = 0 ; i < msg_tpl->test_data_column_count ; i++ )
			{
				printf( "DEBUG - 222 - msg_tpl->test_field[%.*s]\n" , msg_tpl->test_field_len[i] , msg_tpl->test_field[i] );
			}
		}
#endif
		
		msg_tpl->test_data_row_size = TEST_DATA_ROW_SIZE_INIT ;
		
		msg_tpl->test_data = (char **)malloc( sizeof(char*)*msg_tpl->test_data_row_size*msg_tpl->test_data_column_count ) ;
		if( msg_tpl->test_data == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return NULL;
		}
		memset( msg_tpl->test_data , 0x00 , sizeof(char*)*msg_tpl->test_data_row_size*msg_tpl->test_data_column_count );
		
		msg_tpl->test_data_len = (int *)malloc( sizeof(int)*msg_tpl->test_data_row_size*msg_tpl->test_data_column_count ) ;
		if( msg_tpl->test_data_len == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
			_PXFreeTestFieldAndData( msg_tpl );
			fclose( fp );
			return NULL;
		}
		memset( msg_tpl->test_data_len , 0x00 , sizeof(int)*msg_tpl->test_data_row_size*msg_tpl->test_data_column_count );
		
		while(1)
		{
			memset( file_buffer , 0x00 , sizeof(file_buffer) );
			if( fgets( file_buffer , sizeof(file_buffer) , fp ) == NULL )
				break;
			
			if( msg_tpl->test_data_row_count >= msg_tpl->test_data_row_size )
			{
				int		new_test_data_row_size ;
				char		**new_test_data = NULL ;
				int		*new_test_data_len = NULL ;
				
				new_test_data_row_size = msg_tpl->test_data_row_size + TEST_DATA_ROW_SIZE_INCREASE ;
#if _DEBUG
				printf( "DEBUG - 333 - msg_tpl->test_data_row_size[%d] -> [%d]\n" , msg_tpl->test_data_row_size , new_test_data_row_size );
#endif
				
				new_test_data = (char **)realloc( msg_tpl->test_data , sizeof(char*) * new_test_data_row_size * msg_tpl->test_data_column_count ) ;
				if( new_test_data == NULL )
				{
					snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "realloc failed , errno[%d]" , errno );
					_PXFreeTestFieldAndData( msg_tpl );
					fclose( fp );
					return NULL;
				}
				memset( new_test_data+msg_tpl->test_data_row_size*msg_tpl->test_data_column_count , 0x00 , sizeof(char*) * (new_test_data_row_size-msg_tpl->test_data_row_size) * msg_tpl->test_data_column_count );
				msg_tpl->test_data = new_test_data ;
				
				new_test_data_len = (int *)realloc( msg_tpl->test_data_len , sizeof(int) * new_test_data_row_size * msg_tpl->test_data_column_count ) ;
				if( new_test_data_len == NULL )
				{
					snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "realloc failed , errno[%d]" , errno );
					_PXFreeTestFieldAndData( msg_tpl );
					fclose( fp );
					return NULL;
				}
				memset( new_test_data_len+msg_tpl->test_data_row_size*msg_tpl->test_data_column_count , 0x00 , sizeof(int) * (new_test_data_row_size-msg_tpl->test_data_row_size) * msg_tpl->test_data_column_count );
				msg_tpl->test_data_len = new_test_data_len ;
				
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
					snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "strtok for field data failed" );
					_PXFreeTestFieldAndData( msg_tpl );
					fclose( fp );
					return NULL;
				}
				
				if( STRCMP( p , == , "\"\"" ) )
					msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] = strdup( "" ) ;
				else
					msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] = strdup( p ) ;
				if( msg_tpl->test_data[i] == NULL )
				{
					snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "strtok for field data failed" );
					_PXFreeTestFieldAndData( msg_tpl );
					fclose( fp );
					return NULL;
				}
				msg_tpl->test_data_len[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] = strlen(msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i]) ;
#if _DEBUG
				printf( "DEBUG - 333 - msg_tpl->test_data[%d][%d][%.*s]\n" , msg_tpl->test_data_row_count , i , msg_tpl->test_data_len[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] , msg_tpl->test_data[msg_tpl->test_data_row_count*msg_tpl->test_data_column_count+i] );
#endif
			}
			
			msg_tpl->test_data_row_count++;
		}
		
		
		fclose( fp );
	}
	
	nret = PXReadEntireFile( msg_tpl_pathfilename , & (msg_tpl->msg_tpl) , & (msg_tpl->msg_tpl_len) ) ;
	if( nret )
	{
		snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "PXReadEntireFile[%s] failed , errno[%d]" , test_data_pathfilename , errno );
		return NULL;
	}
	
#if _DEBUG
	printf( "DEBUG - 444 - msg_tpl[%.*s]\n" , msg_tpl->msg_tpl_len , msg_tpl->msg_tpl );
#endif
	
	if( msg_tpl->test_field == NULL )
	{
		return msg_tpl;
	}
	
	msg_tpl->substr_lct_count = 1 ;
	if( test_data_pathfilename )
	{
		p = strchr( msg_tpl->msg_tpl , '$' ) ;
		while( p )
		{
			p = strchr( p+1 , '$' ) ;
			if( p == NULL )
			{
				snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "'$' not matched" );
				return NULL;
			}
			
			msg_tpl->substr_lct_count += 2 ;
			
			p = strchr( p+1 , '$' ) ;
		}
	}
	
	msg_tpl->substr_lct_array = (struct PxSubstrLocator *)malloc( sizeof(struct PxSubstrLocator)*msg_tpl->substr_lct_count ) ;
	if( msg_tpl->substr_lct_array == NULL )
	{
		snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
		return NULL;
	}
	memset( msg_tpl->substr_lct_array , 0x00 , sizeof(struct PxSubstrLocator)*msg_tpl->substr_lct_count );
	
	i = 0 ;
	p2 = msg_tpl->msg_tpl ;
	p1 = strchr( p2 , '$' ) ;
	while( p1 )
	{
		msg_tpl->substr_lct_array[i].rpl_offset_ptr = p2 ;
		msg_tpl->substr_lct_array[i].rpl_len = p1 - p2 ;
		msg_tpl->substr_lct_array[i].test_data_column_index = -1 ;
#if _DEBUG
		printf( "DEBUG - 555 - substr_lct1[%d] [%.*s][%d]\n" , i , msg_tpl->substr_lct_array[i].rpl_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->substr_lct_array[i].test_data_column_index );
#endif
		i++;
		
		p2 = strchr( p1+1 , '$' ) ;
		if( p2 == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "'$' not matched" );
			return NULL;
		}
		
		msg_tpl->substr_lct_array[i].rpl_offset_ptr = p1+1 ;
		msg_tpl->substr_lct_array[i].rpl_len = p2-p1-1 ;
		
		for( j = 0 ; j < msg_tpl->test_data_column_count ; j++ )
		{
			if( msg_tpl->test_field_len[j] == msg_tpl->substr_lct_array[i].rpl_len && MEMCMP( msg_tpl->test_field[j] , == , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->test_field_len[j] ) )
			{
				msg_tpl->substr_lct_array[i].test_data_column_index = j ;
				break;
			}
		}
		if( j >= msg_tpl->test_data_column_count )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "var[%.*s] not found in template file[%s]" , msg_tpl->substr_lct_array[i].rpl_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl_pathfilename );
			free( msg_tpl->substr_lct_array );
			return NULL;
		}
		
#if _DEBUG
		printf( "DEBUG - 555 - substr_lct2[%d] [%.*s][%d]\n" , i , msg_tpl->substr_lct_array[i].rpl_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->substr_lct_array[i].test_data_column_index );
#endif
		i++;
		
		p2++;
		p1 = strchr( p2 , '$' ) ;
	}
	
	msg_tpl->substr_lct_array[i].rpl_offset_ptr = p2 ;
	msg_tpl->substr_lct_array[i].rpl_len = msg_tpl->msg_tpl_len - (p2-msg_tpl->msg_tpl) ;
	msg_tpl->substr_lct_array[i].test_data_column_index = -1 ;
#if _DEBUG
	printf( "DEBUG - 555 - substr_lct3[%d] [%.*s][%d]\n" , i , msg_tpl->substr_lct_array[i].rpl_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->substr_lct_array[i].test_data_column_index );
#endif
	i++;
	
	return msg_tpl;
}

char *PXInstaceMessageByRandom( struct PxMessageTemplate *msg_tpl , int *p_msg_ins_len )
{
	int		select_row_index = -1 ;
	int		i ;
	
	if( msg_tpl->test_field == NULL )
	{
		if( p_msg_ins_len )
			(*p_msg_ins_len) = msg_tpl->msg_tpl_len ;
		return msg_tpl->msg_tpl;
	}
	
	select_row_index = Rand( 0 , msg_tpl->test_data_row_count-1 ) ;
#if _DEBUG
	printf( "DEBUG - select_row_index[%d]\n" , select_row_index );
#endif
	
	if( msg_tpl->msg_ins == NULL )
	{
		msg_tpl->msg_ins_bufsize = msg_tpl->msg_tpl_len * 2 ;
		msg_tpl->msg_ins = (char *)malloc( msg_tpl->msg_ins_bufsize ) ;
		if( msg_tpl->msg_ins == NULL )
		{
			snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "malloc failed , errno[%d]" , errno );
			return NULL;
		}
		memset( msg_tpl->msg_ins , 0x00 , msg_tpl->msg_ins_bufsize );
		msg_tpl->msg_ins_len = 0 ;
	}
	
	msg_tpl->msg_ins_len = 0 ;
	for( i = 0 ; i < msg_tpl->substr_lct_count ; i++ )
	{
		if( msg_tpl->substr_lct_array[i].rpl_offset_ptr == NULL )
			break;
		
		if( msg_tpl->msg_ins_len + msg_tpl->substr_lct_array[i].rpl_len >= msg_tpl->msg_ins_bufsize )
		{
			int		new_msg_ins_bufsize ;
			char		*new_msg_ins = NULL ;
			
			new_msg_ins_bufsize = msg_tpl->msg_ins_len + msg_tpl->substr_lct_array[i].rpl_len + 1 ;
			new_msg_ins = (char *)realloc( msg_tpl->msg_ins , new_msg_ins_bufsize ) ;
			if( new_msg_ins == NULL )
			{
				snprintf( _g_message_template_error_desc , sizeof(_g_message_template_error_desc)-1 , "realloc failed , errno[%d]" , errno );
				return NULL;
			}
			msg_tpl->msg_ins_bufsize = new_msg_ins_bufsize ;
			msg_tpl->msg_ins = new_msg_ins ;
		}
		
		if( msg_tpl->substr_lct_array[i].test_data_column_index == -1 )
		{
			memcpy( msg_tpl->msg_ins+msg_tpl->msg_ins_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr , msg_tpl->substr_lct_array[i].rpl_len );
			msg_tpl->msg_ins_len += msg_tpl->substr_lct_array[i].rpl_len ;
#if _DEBUG
			printf( "DEBUG - msg_tpl->msg_tpl substr[%.*s]\n" , msg_tpl->substr_lct_array[i].rpl_len , msg_tpl->substr_lct_array[i].rpl_offset_ptr );
			printf( "DEBUG - msg_tpl->msg_ins[%.*s]\n" , msg_tpl->msg_ins_len , msg_tpl->msg_ins );
#endif
		}
		else
		{
			int		test_data_len ;
			char		*test_data = NULL ;
			
			test_data_len = msg_tpl->test_data_len[msg_tpl->test_data_column_count*select_row_index+msg_tpl->substr_lct_array[i].test_data_column_index] ;
			test_data = msg_tpl->test_data[msg_tpl->test_data_column_count*select_row_index+msg_tpl->substr_lct_array[i].test_data_column_index] ;
			
			memcpy( msg_tpl->msg_ins+msg_tpl->msg_ins_len , test_data , test_data_len );
			msg_tpl->msg_ins_len += test_data_len ;
#if _DEBUG
			printf( "DEBUG - msg_tpl->test_data substr[%.*s]\n" , test_data_len , test_data );
			printf( "DEBUG - msg_tpl->msg_ins[%.*s]\n" , msg_tpl->msg_ins_len , msg_tpl->msg_ins );
#endif
		}
	}
	
	if( p_msg_ins_len )
		(*p_msg_ins_len) = msg_tpl->msg_ins_len ;
	return msg_tpl->msg_ins;
}

void PXFreeMessageTemplate( struct PxMessageTemplate *msg_tpl )
{
	_PXFreeTestFieldAndData( msg_tpl );
	
	if( msg_tpl->msg_tpl )
	{
		free( msg_tpl->msg_tpl );
	}
	
	if( msg_tpl->substr_lct_array )
	{
		free( msg_tpl->substr_lct_array );
	}
	
	if( msg_tpl->msg_ins )
	{
		free( msg_tpl->msg_ins );
	}
	
	free( msg_tpl );
	
	return;
}

char *PXGetMessageTemplateErrorDesc()
{
	return _g_message_template_error_desc;
}

