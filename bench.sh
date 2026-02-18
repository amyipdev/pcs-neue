#!/usr/bin/env sh

nix build .\#pcsbench
rm -f pcsbench-bin
cp result/bin/pcsbench pcsbench-bin
mkdir -p log
case "$1" in
    "drg")
        sudo ./pcsbench-bin --bench=drg | tee log/drg-"$(date +%s)".log
        ;;
    *)
        sudo ./pcsbench-bin | tee log/kern-"$(date +%s)".log
        ;;
esac
