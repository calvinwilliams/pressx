#include "pxagent_api.h"

#include "ib2api.h"

/* run parameter
node app (msg_pathfilename | test_data_pathfilename:msg_tpl_pathfilename) [ file_pathfilename [ file_pathfilename2 ] ]
*/

/* for pxmanager
$ pxmanager --listen-ip 66.88.1.61 --listen-port 9527 -p 1 -t 1 -g pxplugin-ib2.so -m "0000 ACC99999 pxplugin-ib2-ACC99999.test_data:pxplugin-ib2-ACC99999.msg_tpl" -n 1
*/

/* for pxagent
$ pxagent --connect-ip 66.88.1.61 --connect-port 9527
*/

struct PxPluginUserData
{
	struct IB2Env			*ib2_env ;
	
	char				*node ;
	char				*app ;
	char				*msg_pathfilename ;
	char				*test_data_pathfilename ;
	char				*msg_tpl_pathfilename ;
	char				*file_pathfilename ;
	char				*file_pathfilename2 ;
	
	struct PxMessageTemplate	*msg_tpl ;
	char				*file_filename ;
	char				*file_filename2 ;
} ;

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			*p = NULL ;
	char			*p2 = NULL ;
	char			file_filename[ IB2_MAXLEN_FILENAME + 1 ] ;
	char			file_pathfilename[ IB2_MAXLEN_FILENAME + 1 ] ;
	FILE			*fp = NULL ;
	char			command[ 256 + 1 ] ;
	
	int			nret = 0 ;
	
	user_data = (struct PxPluginUserData *)malloc( sizeof(struct PxPluginUserData) ) ;
	if( user_data == NULL )
	{
		printf( "pxplugin-ib2 | malloc PxPluginUserData failed , errno[%d]\n" , errno );
		return -1;
	}
	memset( user_data , 0x00 , sizeof(struct PxPluginUserData) );
	
	hzb_log_init();
	hzb_log_set_category( "press" );
	
	nret = IB2AllocEnvironment( & (user_data->ib2_env) ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2AllocEnvironment failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	nret = IB2LoadClientConfig( user_data->ib2_env , NULL ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2LoadClientConfig failed[%d] , errno[%d]\n" , nret , errno );
		return -1;
	}
	
	p = gettok( GetPxPluginRunParameterPtr(p_pxplugin_ctx) , PRESSX_BLANK_DELIM ) ;
	if( p == NULL )
	{
		printf( "pxplugin-ib2 | expect 'node' in run parameter\n" );
		return -1;
	}
	user_data->node = strdup(p) ;
	if( user_data->node == NULL )
	{
		printf( "pxplugin-ib2 | strdup failed , errno[%d]\n" , errno );
		return -1;
	}
	
	p = gettok( NULL , PRESSX_BLANK_DELIM ) ;
	if( p == NULL )
	{
		printf( "pxplugin-ib2 | expect 'app' in run parameter\n" );
		return -1;
	}
	user_data->app = strdup(p) ;
	if( user_data->app == NULL )
	{
		printf( "pxplugin-ib2 | strdup failed , errno[%d]\n" , errno );
		return -1;
	}
	
	p = gettok( NULL , PRESSX_BLANK_DELIM ) ;
	if( p == NULL )
	{
		printf( "pxplugin-ib2 | expect 'msg_pathfilename' in run parameter\n" );
		return -1;
	}
	p2 = strchr( p , ':' ) ;
	if( p2 == NULL )
	{
		user_data->msg_pathfilename = strdup(p) ;
		user_data->msg_tpl = PXCompileTemplate( NULL , user_data->msg_pathfilename ) ;
		if( user_data->msg_tpl == NULL )
		{
			printf( "pxplugin-ib2 | PXCompileTemplate[(null)][%s] failed[%s]\n" , user_data->msg_pathfilename , PXGetMessageTemplateErrorDesc() );
			return -1;
		}
	}
	else
	{
		user_data->test_data_pathfilename = strndup(p,p2-p) ;
		user_data->msg_tpl_pathfilename = strdup(p2+1) ;
		user_data->msg_tpl = PXCompileTemplate( user_data->test_data_pathfilename , user_data->msg_tpl_pathfilename ) ;
		if( user_data->msg_tpl == NULL )
		{
			printf( "pxplugin-ib2 | PXCompileTemplate[%s][%s] failed[%s]\n" , user_data->test_data_pathfilename , user_data->msg_tpl_pathfilename , PXGetMessageTemplateErrorDesc() );
			return -1;
		}
	}
	
	user_data->file_pathfilename = gettok( NULL , PRESSX_BLANK_DELIM ) ;
	if( user_data->file_pathfilename )
	{
		memset( file_filename , 0x00 , sizeof(file_filename) );
		memset( file_pathfilename , 0x00 , sizeof(file_pathfilename) );
		fp = IB2CreateTempFile( file_filename , file_pathfilename , "w" ) ;
		if( fp == NULL )
		{
			printf( "pxplugin-ib2 | IB2CreateTempFile failed , errno[%d]\n" , errno );
			return -1;
		}
		fclose( fp );
		memset( command , 0x00 , sizeof(command) );
		snprintf( command , sizeof(command)-1 , "cp %s %s" , user_data->file_pathfilename , file_pathfilename );
		nret = system( command ) ;
		if( nret == -1 )
		{
			printf( "pxplugin-ib2 | system[%s] failed[%d] , errno[%d]\n" , command , nret , errno );
			return -1;
		}
		user_data->file_filename = strdup( file_filename ) ;
		if( user_data->file_filename == NULL )
		{
			printf( "pxplugin-ib2 | strdup failed , errno[%d]\n" , errno );
			return -1;
		}
		printf( "pxplugin-ib2 | set tmp file[%s]\n" , user_data->file_filename );
	}
	
	user_data->file_pathfilename2 = gettok( NULL , PRESSX_BLANK_DELIM ) ;
	if( user_data->file_pathfilename2 )
	{
		memset( file_filename , 0x00 , sizeof(file_filename) );
		memset( file_pathfilename , 0x00 , sizeof(file_pathfilename) );
		fp = IB2CreateTempFile( file_filename , file_pathfilename , "w" ) ;
		if( fp == NULL )
		{
			printf( "pxplugin-ib2 | IB2CreateTempFile failed , errno[%d]\n" , errno );
			return -1;
		}
		fclose( fp );
		memset( command , 0x00 , sizeof(command) );
		snprintf( command , sizeof(command)-1 , "cp %s %s" , user_data->file_pathfilename2 , file_pathfilename );
		nret = system( command ) ;
		if( nret == -1 )
		{
			printf( "pxplugin-ib2 | system[%s] failed[%d] , errno[%d]\n" , command , nret , errno );
			return -1;
		}
		user_data->file_filename2 = strdup( file_filename ) ;
		if( user_data->file_filename2 == NULL )
		{
			printf( "pxplugin-ib2 | strdup failed , errno[%d]\n" , errno );
			return -1;
		}
		printf( "pxplugin-ib2 | set tmp file2[%s]\n" , user_data->file_filename2 );
	}
	
	SetPxPluginUserData( p_pxplugin_ctx , user_data );
	
	return 0;
}

funcRunPxPlugin RunPxPlugin ;
int RunPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			*msg = NULL ;
	int			msg_len ;
	struct IB2PackageInfo	msg_info ;
	struct IB2PackageInfo	file_info ;
	struct COM_comm_confirm	comm_confirm ;
	long			timeout ;
	
	int			nret = 0 ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	IB2DeleteAllPackages( user_data->ib2_env );
	
	nret = IB2ConnectToServerByServerNode( user_data->ib2_env , user_data->node ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2ConnectToServerByServerNode[%s] failed[%d]\n" , user_data->node , nret );
		return -1;
	}
	
	/*
	{
		int	onoff = 1 ;
		setsockopt( user_data->ib2_env->clisock , SOL_SOCKET , SO_REUSEADDR , (void *) & onoff , sizeof(int) );
	}
	*/
	
	msg = PXInstaceMessageByRandom( user_data->msg_tpl , & msg_len ) ;
	if( msg == NULL )
	{
		printf( "pxplugin-ib2 | PXInstaceMessageByRandom failed[%s]\n" , PXGetMessageTemplateErrorDesc() );
		return -1;
	}
	
	memset( & msg_info , 0x00 , sizeof(struct IB2PackageInfo) );
	strcpy( msg_info.struct_type , "IB2MSG_STRUCTTYPE" );
	strcpy( msg_info.id , "IB2MSG_ID" );
	nret = IB2AddMessage( user_data->ib2_env , & msg_info , msg , msg_len ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2AddMessage failed[%d]\n" , nret );
		return -1;
	}
	
	if( user_data->file_filename )
	{
		memset( & msg_info , 0x00 , sizeof(struct IB2PackageInfo) );
		strcpy( msg_info.struct_type , "IB2FILE_STRUCTTYPE" );
		strcpy( msg_info.id , "IB2FILE_ID" );
		nret = IB2AddFile( user_data->ib2_env , & file_info , user_data->file_filename ) ;
		if( nret )
		{
			printf( "pxplugin-ib2 | IB2AddFile failed[%d]\n" , nret );
			return -1;
		}
	}
	
	if( user_data->file_filename2 )
	{
		memset( & msg_info , 0x00 , sizeof(struct IB2PackageInfo) );
		strcpy( msg_info.struct_type , "IB2FILE2_STRUCTTYPE" );
		strcpy( msg_info.id , "IB2FILE2_ID" );
		nret = IB2AddFile( user_data->ib2_env , & file_info , user_data->file_filename2 ) ;
		if( nret )
		{
			printf( "pxplugin-ib2 | IB2AddFile failed[%d]\n" , nret );
			return -1;
		}
	}
	
	memset( & comm_confirm , 0x00 , sizeof(struct COM_comm_confirm) );
	nret = IB2SendRequest( user_data->ib2_env , user_data->app , 0 , & timeout , & comm_confirm ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2SendRequest failed[%d] , [%ld][%s] , [%.*s]\n" , nret , comm_confirm.response_code , comm_confirm.response_desc , msg_len , msg );
		return -1;
	}
	
	memset( & comm_confirm , 0x00 , sizeof(struct COM_comm_confirm) );
	nret = IB2ReceiveResponse( user_data->ib2_env , & timeout , & comm_confirm ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2ReceiveResponse failed[%d] , [%ld][%s] , [%.*s]\n" , nret , comm_confirm.response_code , comm_confirm.response_desc , msg_len , msg );
		return -1;
	}
	
	IB2DisconnectFromServer( user_data->ib2_env );
	
	return 0;
}

funcCleanPxPlugin CleanPxPlugin ;
int CleanPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	
	user_data = GetPxPluginUserData( p_pxplugin_ctx ) ;
	
	IB2FreeEnvironment( & (user_data->ib2_env) );
	
	if( user_data->node )
		free( user_data->node );
	if( user_data->app )
		free( user_data->app );
	if( user_data->msg_pathfilename )
		free( user_data->msg_pathfilename );
	if( user_data->test_data_pathfilename )
		free( user_data->test_data_pathfilename );
	if( user_data->msg_tpl_pathfilename )
		free( user_data->msg_tpl_pathfilename );
	if( user_data->file_pathfilename )
		free( user_data->file_pathfilename );
	if( user_data->file_pathfilename2 )
		free( user_data->file_pathfilename2 );
	
	PXFreeMessageTemplate( user_data->msg_tpl );
	if( user_data->file_filename )
		free( user_data->file_filename );
	if( user_data->file_filename2 )
		free( user_data->file_filename2 );
	
	free( user_data );
	
	return 0;
}

