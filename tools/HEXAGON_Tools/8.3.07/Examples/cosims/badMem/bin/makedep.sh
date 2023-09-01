#!/bin/sh
DIR="$1"
shift 1
case "$DIR" in
"" | ".")
	gcc -MM "$@" | sed -e 's@^\(.*\)\.o:@\1.o:@' | sed -e "s@//@/@g"
	;;
*)
	gcc -MM "$@" | sed -e "s@^\(.*\)\.o:@$DIR/\1.o:@" | sed -e "s@//@/@g"
	;;
esac

