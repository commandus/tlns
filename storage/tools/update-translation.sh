#!/bin/bash
TEMPLATE=po/lorawan-storage.pot
for f in $(ls po/*.po) ; do
  regex="\/(.*)\.([a-z][a-z])_..\.UTF-8\.po"
  if [[ $f =~ $regex ]]; then
    fn="${BASH_REMATCH[1]}"
    code="${BASH_REMATCH[2]}"

    case $fn in
      'lorawan-identity-query') FM='cli-query-main.cpp';;
      'lorawan-identity-service') FM='cli-main.cpp';;
      'lorawan-identity-direct') FM='cli-query-plugin-main.cpp';;
      'lorawan-identity-print') FM='cli-print.cpp';;
      'lorawan-tag') FM='cli-tag.cpp';;
      
      
      *) FM='*';;
    esac

    xgettext -k_ -o $TEMPLATE $FM
    echo -n Merge $fn $FM ${code} ..
    msgmerge -U $f $TEMPLATE
    mkdir -p locale/${code}/LC_MESSAGES
    msgfmt -o locale/${code}/LC_MESSAGES/$fn.mo $f
  fi
  echo sudo cp locale/${code}/LC_MESSAGES/\*.mo /usr/share/locale/${code}/LC_MESSAGES/
done
