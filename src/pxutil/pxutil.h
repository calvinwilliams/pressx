#ifndef _H_PXUTIL_
#define _H_PXUTIL_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stddef.h>
#include <pwd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <values.h>
#include <dlfcn.h>

char *strcasestr(const char *haystack, const char *needle);

#include "list.h"

#ifndef MAX
#define MAX(_a_,_b_) ( (_a_)>(_b_)?(_a_):(_b_) )
#endif

#ifndef MIN
#define MIN(_a_,_b_) ( (_a_)<(_b_)?(_a_):(_b_) )
#endif

#ifndef STRCMP
#define STRCMP(_a_,_C_,_b_) ( strcmp(_a_,_b_) _C_ 0 )
#define STRNCMP(_a_,_C_,_b_,_n_) ( strncmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef STRICMP
#if ( defined _WIN32 )
#define STRICMP(_a_,_C_,_b_) ( _stricmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( _strnicmp(_a_,_b_,_n_) _C_ 0 )
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ )
#define STRICMP(_a_,_C_,_b_) ( strcasecmp(_a_,_b_) _C_ 0 )
#define STRNICMP(_a_,_C_,_b_,_n_) ( strncasecmp(_a_,_b_,_n_) _C_ 0 )
#endif
#endif

#ifndef MEMCMP
#define MEMCMP(_a_,_C_,_b_,_n_) ( memcmp(_a_,_b_,_n_) _C_ 0 )
#endif

#ifndef STRISTR
#if ( defined _WIN32 )
#define STRISTR		strstr /* ÒÔºó¸Ä */
#elif ( defined __unix ) || ( defined _AIX ) || ( defined __linux__ )
#define STRISTR		strcasestr
#endif
#endif

#ifndef STRMINCMP
#define STRMINCMP(_str_,_C_,_substr_)	( strncmp(_str_,_substr_,strlen(_substr_)) _C_ 0 )
#endif

#define VAL_TIMEVAL(_tv1_,_tv2_) \
	(_tv1_).tv_sec = (_tv2_).tv_sec ; \
	(_tv1_).tv_usec = (_tv2_).tv_usec ; \

#define MIN_VAL_TIMEVAL(_tv1_,_tv2_) \
	if( (_tv2_).tv_sec < (_tv1_).tv_sec ) \
	{ \
		(_tv1_).tv_sec = (_tv2_).tv_sec ; \
		(_tv1_).tv_usec = (_tv2_).tv_usec ; \
	} \
	else if( (_tv2_).tv_sec == (_tv1_).tv_sec && (_tv2_).tv_usec < (_tv1_).tv_usec ) \
	{ \
		(_tv1_).tv_sec = (_tv2_).tv_sec ; \
		(_tv1_).tv_usec = (_tv2_).tv_usec ; \
	} \

#define MAX_VAL_TIMEVAL(_tv1_,_tv2_) \
	if( (_tv2_).tv_sec > (_tv1_).tv_sec ) \
	{ \
		(_tv1_).tv_sec = (_tv2_).tv_sec ; \
		(_tv1_).tv_usec = (_tv2_).tv_usec ; \
	} \
	else if( (_tv2_).tv_sec == (_tv1_).tv_sec && (_tv2_).tv_usec > (_tv1_).tv_usec ) \
	{ \
		(_tv1_).tv_sec = (_tv2_).tv_sec ; \
		(_tv1_).tv_usec = (_tv2_).tv_usec ; \
	} \

#define DIFF_TIMEVAL(_tv_diff_,_tv1_,_tv2_) \
	(_tv_diff_).tv_sec = (_tv2_).tv_sec - (_tv1_).tv_sec ; \
	(_tv_diff_).tv_usec = (_tv2_).tv_usec - (_tv1_).tv_usec ; \
	while( (_tv_diff_).tv_usec < 0 ) \
	{ \
		(_tv_diff_).tv_usec += 1000000 ; \
		(_tv_diff_).tv_sec--; \
	} \

struct NetAddress
{
	char			ip[ 15 + 1 ] ;
	int			port ;
	int			sock ;
	struct sockaddr_in	addr ;
	
	struct sockaddr_in	local_addr ;
	char			local_ip[ 15 + 1 ] ;
	int			local_port ;
	
	struct sockaddr_in	remote_addr ;
	char			remote_ip[ 15 + 1 ] ;
	int			remote_port ;
} ;

#define SETNETADDRESS(_netaddr_) \
	memset( & ((_netaddr_).addr) , 0x00 , sizeof(struct sockaddr_in) ); \
	(_netaddr_).addr.sin_family = AF_INET ; \
	if( (_netaddr_).ip[0] == '\0' ) \
		(_netaddr_).addr.sin_addr.s_addr = INADDR_ANY ; \
	else \
		(_netaddr_).addr.sin_addr.s_addr = inet_addr((_netaddr_).ip) ; \
	(_netaddr_).addr.sin_port = htons( (unsigned short)((_netaddr_).port) );

#define GETNETADDRESS(_netaddr_) \
	strcpy( (_netaddr_).ip , inet_ntoa((_netaddr_).addr.sin_addr) ); \
	(_netaddr_).port = (int)ntohs( (_netaddr_).addr.sin_port ) ;

#define GETNETADDRESS_LOCAL(_netaddr_) \
	{ \
	socklen_t	socklen = sizeof(struct sockaddr) ; \
	int		nret = 0 ; \
	nret = getsockname( (_netaddr_).sock , (struct sockaddr*)&((_netaddr_).local_addr) , & socklen ) ; \
	if( nret == 0 ) \
	{ \
		strcpy( (_netaddr_).local_ip , inet_ntoa((_netaddr_).local_addr.sin_addr) ); \
		(_netaddr_).local_port = (int)ntohs( (_netaddr_).local_addr.sin_port ) ; \
	} \
	}

#define GETNETADDRESS_REMOTE(_netaddr_) \
	{ \
	socklen_t	socklen = sizeof(struct sockaddr) ; \
	int		nret = 0 ; \
	nret = getpeername( (_netaddr_).sock , (struct sockaddr*)&((_netaddr_).remote_addr) , & socklen ) ; \
	if( nret == 0 ) \
	{ \
		strcpy( (_netaddr_).remote_ip , inet_ntoa((_netaddr_).remote_addr.sin_addr) ); \
		(_netaddr_).remote_port = (int)ntohs( (_netaddr_).remote_addr.sin_port ) ; \
	} \
	}

#define PRESSX_BLANK_DELIM		" \t\f\r\n"

char *gettok( char *str , const char *delim );
char *TrimEnter( char *str );

char *GetUsernamePtr();

int writen( int sock , char *send_buffer , int send_len , int *p_sent_len );
int readn( int sock , char *recv_buffer , int recv_len , int *p_received_len );

#define PRESSX_MAXLEN_USER_NAME		64

struct PxRegisteMessage
{
	char		user_name[ PRESSX_MAXLEN_USER_NAME + 1 ] ;
} ;

#define PRESSX_MAXLEN_RUN_PLUGIN	256
#define PRESSX_MAXLEN_RUN_PARAMETER	256

struct PxRunPressingMessage
{
	unsigned int	process_count ;
	unsigned int	thread_count ;
	unsigned int	run_count ;
	char		run_plugin[ PRESSX_MAXLEN_RUN_PLUGIN + 1 ] ;
	char		run_parameter[ PRESSX_MAXLEN_RUN_PARAMETER + 1 ] ;
} ;

struct PxPerformanceStatMessage
{
	struct timeval	total_run_elapse ;
	struct timeval	min_delay_elapse ;
	struct timeval	max_delay_elapse ;
} ;

#endif

