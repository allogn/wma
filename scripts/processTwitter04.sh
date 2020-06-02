#!/bin/bash

for d in $(find /q/storage/alogins/fcla_data/real/twitter/04_month -maxdepth 5 -type d)
do
  if [ -d "$d" ]; then
    echo "$d"
    #if [ -f $d/all_twits.json ]; then
    #    echo "File found!"
    #fi
    for filename in $d/*.bz2; do
      [ -f "$filename" ] || break
      echo "$filename"
      bzip2 -d $filename
    done
    echo "Merging..."
    python $FCLA_ROOT/scripts/collectTwitter.py $d
    mv $d/all_twits.json $d/tmp
    rm $d/*.json
    mv $d/tmp $d/all_twits.json
  fi
done
