#!/bin/bash 

prefix="${1:-/usr/local/LKC}"
BINS="bin/LKC"
LIBS="lib/LKC.jar lib/liblkc.a"

main()
{
    if ! [[ -f lib/lkc.jar && -f lib/liblkc.a ]]
    then
        echo "lib/lkc.jar and lib/liblkc.a do not exist.  Build it first" 1>&2
        exit 1
    fi
    echo "prefix=$prefix"
    invoke mkdir -p "$prefix/bin"
    invoke install -m755 $BINS "$prefix/bin"
    invoke mkdir -p "$prefix/lib"
    invoke cp $LIBS "$prefix/lib"
    invoke rm -rf "$prefix/import"
    invoke cp -r import "$prefix/import"
    echo "lkc successfully installed as $prefix/bin/lkc"
}

invoke()
{
    echo "$@"
    if ! "$@"
    then
        echo "install failed." 1>&2
        exit 1
    fi
}
main "$@"
