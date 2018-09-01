OS=`uname | awk '{print $1}'`
VERSION=`pxmanager -v| awk '{print $2}' | cut -c2-`

BINFILES="	bin/pxmanager
		bin/pxagent
"

LIBFILES="	lib/libpxutil.so
		lib/libpxagent_api.so
"

PLUGINFILES="	so/pxplugin-*.so
"

SHFILES="	shbin/pressx_pack_bin.sh
"

cd $HOME
tar cvzf pressx-${OS}-${VERSION}-bin.tar.gz $BINFILES $LIBFILES $PLUGINFILES $SHFILES

