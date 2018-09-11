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
			pxplugin-tcp-connect.c \
			pxplugin-http.c \
			fasterhttp.c \
			pxplugin-http-fasterhttp.c \
			pxplugin-http-fasterhttp-IOMP.c \

###### 目标文件、安装目录配置区
include makeinstall
LIB		=	\
			pxplugin-tcp-connect.so \
			pxplugin-http.so \
			pxplugin-http-fasterhttp.so \
			pxplugin-http-fasterhttp-IOMP.so \

#			pxplugin-http-fasterhttp-IOMP.so \

LIBINST		=	$(HOME)/so

###### 编译选项
CFLAGS		=	$(_CFLAGS) \
			-I$(HOME)/include/pressx \

CFLAGS_IB2	=	$(CFLAGS) \
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

LFLAGS_IB2	=	$(LFLAGS) \
			-lib2api \
			-lib2core \
			-lib2fmd \
			-lib2ssd \
			-lib2util \
			-lhzb_log \

###### 额外宏定义区
CLEAN_ADDITION	=

PREMAKE		=

###### 加载mktpl模板库
#@ make_all
#@ make_clean
#@ make_install
#@ make_install_LIBINST
#@ make_uninstall
#@ make_uninstall_LIBINST
include $(MKTPL2_HOME)/makeobj_$(MKTPL2_OS).inc

###### 目标文件依赖关系
pxplugin-ib2.o	:	pxplugin-ib2.c
	$(CC) $(CFLAGS_IB2) -c pxplugin-ib2.c

pxplugin-ib2.so	:	pxplugin-ib2.o
	$(CC) -o $@ pxplugin-ib2.o $(SOFLAGS) $(LFLAGS_IB2)

pxplugin-ib2_install	:	pxplugin-ib2.so
	@if [ ! -d "$(LIBINST)" ] ; then \
		echo $(MKDIR) $(LIBINST) ; \
		$(MKDIR) $(LIBINST) ; \
	fi
	@for FILE in `echo $(LIB)` ; do \
		if [ -f $(LIBINST)/pxplugin-ib2.so ] ; then \
			echo $(RM) $(LIBINST)/pxplugin-ib2.so ; \
			$(RM) $(LIBINST)/pxplugin-ib2.so ; \
		fi ; \
		echo $(CP) pxplugin-ib2.so $(LIBINST)/ ; \
		$(CP) pxplugin-ib2.so $(LIBINST)/ ; \
	done

pxplugin-tcp-connect.so	:	pxplugin-tcp-connect.o
	$(CC) -o $@ pxplugin-tcp-connect.o $(SOFLAGS) $(LFLAGS)

pxplugin-http.so	:	pxplugin-http.o
	$(CC) -o $@ pxplugin-http.o $(SOFLAGS) $(LFLAGS)

pxplugin-http-fasterhttp.so	:	pxplugin-http-fasterhttp.o fasterhttp.o
	$(CC) -o $@ pxplugin-http-fasterhttp.o fasterhttp.o $(SOFLAGS) $(LFLAGS)

pxplugin-http-fasterhttp-IOMP.so	:	pxplugin-http-fasterhttp-IOMP.o fasterhttp.o
	$(CC) -o $@ pxplugin-http-fasterhttp-IOMP.o fasterhttp.o $(SOFLAGS) $(LFLAGS)

