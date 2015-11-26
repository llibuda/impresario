#! /bin/sh

makeAbsolute() {
    case $1 in
        /*)
            # already absolute, return it
            echo "$1"
            ;;
        *)
            # relative, prepend $2 made absolute
            echo `makeAbsolute "$2" "$PWD"`/"$1" | sed 's,/\.$,,'
            ;;
    esac
}

me=`which "$0"` # Search $PATH if necessary
if test -L "$me"; then
    # Try readlink(1)
    readlink=`type readlink 2>/dev/null` || readlink=
    if test -n "$readlink"; then
        # We have readlink(1), so we can use it. Assuming GNU readlink (for -f).
        me=`readlink -nf "$me"`
    else
        # No readlink(1), so let's try ls -l
        me=`ls -l "$me" | sed 's/^.*-> //'`
        base=`dirname "$me"`
        me=`makeAbsolute "$me" "$base"`
    fi
fi

bindir=`dirname "$me"`
libdir=`cd "$bindir/lib" ; pwd`

# Try to find user's Impresario.xml file
config=`find ~/.config/ -path "*/.*Impresario.xml"`
depdirs=""
if test -n "$config"; then
    # If config file exists, try to read line with directories containing dependent shared objects
    depdirs=`sed -nr 's/^.*<DepLibs>(.*)<\/DepLibs>/\1/p' "$config"`
    depdirlist=`sed -nr 's/^.*<DepLibs>(.*)<\/DepLibs>/\1/p' "$config" | sed -nr 's/\|/:/gp'`
    if test -n "$depdirlist"; then
        depdirs="$depdirlist"
    fi
fi

# Extend path for shared objects for this process
LD_LIBRARY_PATH=$libdir:$depdirs${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}
export LD_LIBRARY_PATH
#echo $LD_LIBRARY_PATH
# Change to Impresario's executable directory so that initialization works properly
cd $bindir
# Run Impresario main process
exec "$bindir/Impresario" ${1+"$@"}