# 此文件由makeobj.sh自动生成
############################################################
# 项目名 : 
# 模块名 : 
# 备  注 : 
############################################################

###### 源文件配置区
#@ c_FILE
c_FILE		=	\
			pxplugin-ib2.c \

###### 目标文件、安装目录配置区
include makeinstall
LIB		=	pxplugin-ib2.so
LIBINST		=	$(HOME)/so

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(HOME)/include/pressx_api \
			-I$(HOME)/include/ib2util \
			-I$(HOME)/include/ib2ssd \
			-I$(HOME)/include/ib2fmd \
			-I$(HOME)/include/ib2core \
			-I$(HOME)/include/ib2api \
			-I$(HOME)/include/ib1 \

###### 链接选项
LFLAGS		=	$(_LFLAGS) \
			-L$(HOME)/lib \
			-lpxagent_api \

###### 额外宏定义区
CLEAN_ADDITION	=

PREMAKE		=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_install_DFTHDERINST
#@ make_uninstall
#@ make_uninstall_LIBINST
#@ make_uninstall_DFTHDERINST
include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### 目标文件依赖关系
pxplugin-ib2.so		:	$(c_FILE_o)
	$(CC) -o $@ $(c_FILE_o) $(SOFLAGS) $(LFLAGS)

