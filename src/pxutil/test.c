#include "pxutil.h"

int main( int argc , char *argv[] )
{
	char				*test_data_pathfilename = NULL ;
	char				*msg_tpl_pathfilename = NULL ;
	int				instance_count ;
	int				instance_index ;
	struct PxMessageTemplate	*msg_tpl = NULL ;
	char				*msg_ins = NULL ;
	int				msg_ins_len ;
	
	int				nret = 0 ;
	
	if( argc != 1 + 3 )
	{
		printf( "USAGE : test test_data_pathfilename msg_tpl_pathfilename instance_count\n" );
		return 7;
	}
	
	test_data_pathfilename = argv[1] ;
	msg_tpl_pathfilename = argv[2] ;
	instance_count = atoi(argv[3]) ;
	
	msg_tpl = PXCreateMessageTemplate() ;
	if( msg_tpl == NULL )
	{
		printf( "PXCreateMessageTemplate failed\n" );
		return 1;
	}
	else
	{
		printf( "PXCreateMessageTemplate ok\n" );
	}
	
	nret = PXLoadTestData( msg_tpl , test_data_pathfilename ) ;
	if( nret )
	{
		printf( "PXLoadTestData[%s] failed[%d]\n" , test_data_pathfilename , nret );
		return 1;
	}
	else
	{
		printf( "PXLoadTestData[%s] ok\n" , test_data_pathfilename );
	}
	
	nret = PXLoadMessageTemplate( msg_tpl , msg_tpl_pathfilename ) ;
	if( nret )
	{
		printf( "PXLoadMessageTemplate[%s] failed[%d]\n" , msg_tpl_pathfilename , nret );
		return 1;
	}
	else
	{
		printf( "PXLoadMessageTemplate[%s] ok\n" , msg_tpl_pathfilename );
	}
	
	for( instance_index = 0 ; instance_index < instance_count ; instance_index++ )
	{
		nret = PXInstaceMessageByRandom( msg_tpl ) ;
		if( nret )
		{
			printf( "PXInstaceMessageByRandom failed[%d]\n" , nret );
			return 1;
		}
		
		msg_ins = PXGetMessagePtr( msg_tpl , & msg_ins_len ) ;
		if( msg_ins == NULL )
		{
			printf( "PXGetMessagePtr failed\n" );
			return 1;
		}
		
		printf( "--- instance_index[%d] --------- msg_ins[%.*s]\n" , instance_index , msg_ins_len , msg_ins );
	}
	
	PXDestroyMessageTemplate( msg_tpl );
	
	return 0;
}

