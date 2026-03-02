#!/usr/bin/env zsh
set -euo pipefail

mkdir -p .tmp
rm -f .tmp/lisaem-xenix-console.txt
./bin/LisaEm.app/Contents/MacOS/lisaem.sh
rg -a -n "\[xenix-pf-ff\]|\[xenix-pf-reset\]|\[xenix-pf\]|\[xenix-trap0-ram\]|\[xenix-trap0-candraw\]|\[xenix-trap0-cand\]|\[xenix-trap0-badread\]|\[xenix-trap0-buf\]|\[xenix-trap0\].*sys=4\\b" .tmp/lisaem-xenix-console.txt | head -n 700 || true
echo "---- trap0-ram payload lines ----"
rg -a -n "\[xenix-trap0-ram\]" -A 1 .tmp/lisaem-xenix-console.txt | head -n 500 || true
echo "---- install text milestones ----"
rg -a -n "WARNING:|Do you want to continue|Enter size of hard disk|Making file system|/mdec/pfbootx|hdinit|upgrade" .tmp/lisaem-xenix-console.txt | head -n 200 || true
echo "---- log tail ----"
tail -n 250 .tmp/lisaem-xenix-console.txt
