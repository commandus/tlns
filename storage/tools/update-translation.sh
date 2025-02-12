#!/bin/bash
TEMPLATE=po/lorawan-storage.pot
for f in $(ls po/*.po) ; do
  regex="\/(.*)\.([a-z][a-z])_..\.UTF-8\.po"
  if [[ $f =~ $regex ]]; then
    fn="${BASH_REMATCH[1]}"
    code="${BASH_REMATCH[2]}"

    case $fn in
      'gateway-config2cpp') FM='util/gateway-config2cpp.cpp';;
      'regional-parameters2cpp') FM='util/regional-parameters2cpp.cpp';;
      'tlns-check') FM='cli-main-check.cpp';;
      'lorawan-identity-direct') FM='storage/cli-query-plugin-main.cpp';;
      'lorawan-identity-print') FM='storage/cli-print.cpp';;
      'lorawan-identity-query') FM='storage/cli-query-main.cpp';;
      'lorawan-identity-service') FM='storage/cli-main.cpp';;
      'lorawan-tag') FM='storage/cli-tag.cpp';;
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
