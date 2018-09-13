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
	
	if( argc != 1 + 3 )
	{
		printf( "USAGE : test (test_data_pathfilename|\"\") msg_tpl_pathfilename instance_count\n" );
		return 7;
	}
	
	test_data_pathfilename = argv[1] ;
	if( test_data_pathfilename[0] == '\0' )
		test_data_pathfilename = NULL ;
	msg_tpl_pathfilename = argv[2] ;
	instance_count = atoi(argv[3]) ;
	
	msg_tpl = PXCompileTemplate( test_data_pathfilename , msg_tpl_pathfilename ) ;
	if( msg_tpl == NULL )
	{
		printf( "PXCompileTemplate[%s][%s] failed\n" , test_data_pathfilename , msg_tpl_pathfilename );
		return 1;
	}
	else
	{
		printf( "PXCompileTemplate[%s][%s] ok\n" , test_data_pathfilename , msg_tpl_pathfilename );
	}
	
	for( instance_index = 0 ; instance_index < instance_count ; instance_index++ )
	{
		msg_ins = PXInstaceMessageByRandom( msg_tpl , & msg_ins_len ) ;
		if( msg_ins == NULL )
		{
			printf( "PXInstaceMessageByRandom failed\n" );
			return 1;
		}
		
		printf( "--- instance_index[%d] --------- msg_ins[%.*s]\n" , instance_index , msg_ins_len , msg_ins );
	}
	
	PXFreeMessageTemplate( msg_tpl );
	
	return 0;
}

