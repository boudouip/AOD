#! /bin/bash
if [ $# -ne 1 ]
then
    echo "Usage: $0 benchmarks_main_folder"
    exit 1
fi

if [ ! -d "$1" ]
then
    echo "$1 inaccessible"
    exit 1
fi

BINDIR=bin
BENCHDIR=$(cd "$1/" && pwd)
cd "$(dirname "$0" )"

make binary
for i in {1..6}
do
    echo "Benchmark $i"
    /usr/bin/time -f "user time : %U" $BINDIR/computePatchOpt "$BENCHDIR/benchmark_0$i/source" "$BENCHDIR/benchmark_0$i/target" > patch
    $BINDIR/applyPatch patch "$BENCHDIR/benchmark_0$i/source" > target
    diff "$BENCHDIR/benchmark_0$i/target" target > /dev/null
    if [ $? -eq 0 ]
    then
        echo "Réussi"
    else
        echo "Echoué"
    fi
    echo "-----------"
done

rm target patch

exit 0
