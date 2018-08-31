# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			main.c \
			agent.c \
			comm.c \
			app.c \

c_FILE2		=	\
			pxagent_api.c \

###### 目标文件、安装目录配置区
include makeinstall
BIN		=	pxagent
BININST		=	$(_BININST)
HDER		=	pxagent_api.h
HDERINST	=	$(_HDERINST)/pressx
LIB		=	libpxagent_api.so
LIBINST		=	$(_LIBINST)

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(HOME)/include/pressx \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(HOME)/lib \
			-lpxutil \
			-lpthread \
			-ldl \

###### 额外宏定义区
CLEAN_ADDITION	=

PREMAKE		=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_BININST
#@ make_install_HDERINST
#@ make_install_DFTHDERINST
#@ make_install_LIBINST
#@ make_uninstall
#@ make_uninstall_BININST
#@ make_uninstall_HDERINST
#@ make_uninstall_DFTHDERINST
#@ make_uninstall_LIBINST
include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### 目标文件依赖关系
pxagent		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(LFLAGS)

libpxagent_api.so	:	pxagent_api.o
	$(CC) $(SOFLAGS) -o $@ $(c_FILE2_o) $(LFLAGS)

