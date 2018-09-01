#include "pxagent_api.h"

#include "ib2api.h"

/* run command format
node app msg_pathfilename [ file_pathfilename [ file_pathfilename2 ] ]
*/

struct PxPluginUserData
{
	struct IB2Env		*ib2_env ;
	
	char			*node ;
	char			*app ;
	char			*msg_pathfilename ;
	char			*file_pathfilename ;
	char			*file_pathfilename2 ;
	
	char			*msg_tpl ;
	int			msg_tpl_len ;
	char			*file_filename ;
	char			*file_filename2 ;
} ;

funcInitPxPlugin InitPxPlugin ;
int InitPxPlugin( struct PxPluginContext *p_pxplugin_ctx )
{
	struct PxPluginUserData	*user_data = NULL ;
	char			*p = NULL ;
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
		printf( "pxplugin-ib2 | expect 'app' in run command\n" );
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
		printf( "pxplugin-ib2 | expect 'msg_pathfilename' in run command\n" );
		return -1;
	}
	user_data->msg_pathfilename = strdup(p) ;
	if( user_data->msg_pathfilename == NULL )
	{
		printf( "pxplugin-ib2 | strdup failed , errno[%d]\n" , errno );
		return -1;
	}
	
	fp = fopen( user_data->msg_pathfilename , "r" ) ;
	if( fp == NULL )
	{
		printf( "pxplugin-ib2 | fopen[%s] failed , errno[%d]\n" , user_data->msg_pathfilename , errno );
		return -1;
	}
	fseek( fp , 0 , SEEK_END );
	user_data->msg_tpl_len = ftell( fp ) ;
	fseek( fp , 0 , SEEK_SET );
	user_data->msg_tpl = (char *)malloc( user_data->msg_tpl_len+1 ) ;
	nret = fread( user_data->msg_tpl , user_data->msg_tpl_len , 1 , fp ) ;
	if( nret != 1 )
	{
		printf( "pxplugin-ib2 | fread[%s] failed , errno[%d]\n" , user_data->msg_pathfilename , errno );
		fclose( fp );
		return -1;
	}
	user_data->msg_tpl[user_data->msg_tpl_len] = '\0' ;
	fclose( fp );
	printf( "pxplugin-ib2 | load msg tpl file[%s] ok , len[%d]\n" , user_data->msg_pathfilename , user_data->msg_tpl_len );
	
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
	
	memset( & msg_info , 0x00 , sizeof(struct IB2PackageInfo) );
	strcpy( msg_info.struct_type , "IB2MSG_STRUCTTYPE" );
	strcpy( msg_info.id , "IB2MSG_ID" );
	nret = IB2AddMessage( user_data->ib2_env , & msg_info , user_data->msg_tpl , user_data->msg_tpl_len ) ;
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
		printf( "pxplugin-ib2 | IB2SendRequest failed[%d] , [%ld][%s]\n" , nret , comm_confirm.response_code , comm_confirm.response_desc );
		return -1;
	}
	
	memset( & comm_confirm , 0x00 , sizeof(struct COM_comm_confirm) );
	nret = IB2ReceiveResponse( user_data->ib2_env , & timeout , & comm_confirm ) ;
	if( nret )
	{
		printf( "pxplugin-ib2 | IB2ReceiveResponse failed[%d] , [%ld][%s]\n" , nret , comm_confirm.response_code , comm_confirm.response_desc );
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
	if( user_data->file_pathfilename )
		free( user_data->file_pathfilename );
	if( user_data->file_pathfilename2 )
		free( user_data->file_pathfilename2 );
	
	if( user_data->msg_tpl )
		free( user_data->msg_tpl );
	if( user_data->file_filename )
		free( user_data->file_filename );
	if( user_data->file_filename2 )
		free( user_data->file_filename2 );
	
	free( user_data );
	
	return 0;
}

