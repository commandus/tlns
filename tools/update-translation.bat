@ECHO OFF
REM po\gateway-config2cpp.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot util/gateway-config2cpp.cpp
msgmerge -U po\gateway-config2cpp.ru_RU.UTF-8.po po\tlns.pot
msgfmt -o locale\gateway-config2cpp.ru\LC_MESSAGES\gateway-config2cpp.mo po\gateway-config2cpp.ru_RU.UTF-8.po

REM regional-parameters2cpp.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot util/regional-parameters2cpp.cpp
msgmerge -U po\regional-parameters2cpp.ru_RU.UTF-8.po po\tlns.pot
msgfmt -o locale\regional-parameters2cpp.ru\LC_MESSAGES\regional-parameters2cpp.mo po\regional-parameters2cpp.ru_RU.UTF-8.po

REM tlns-check.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot cli-main-check.cpp
msgmerge -U po\tlns-check.ru_RU.UTF-8.po po\tlns.pot
msgfmt -o locale\tlns-check.ru\LC_MESSAGES\tlns-check.mo po\tlns-check.ru_RU.UTF-8.po

REM lorawan-identity-direct.ru_RU.UTF-8.po 
xgettext -k_ -o po\tlns.pot storage/cli-query-plugin-main.cpp
msgmerge -U po\lorawan-identity-direct.ru_RU.UTF-8.po  po\tlns.pot
msgfmt -o locale\lorawan-identity-direct.ru\LC_MESSAGES\lorawan-identity-direct.mo po\lorawan-identity-direct.ru_RU.UTF-8.po

REM lorawan-identity-print.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot storage/cli-print.cpp
msgmerge -U po\lorawan-identity-print.ru_RU.UTF-8.po  po\tlns.pot
msgfmt -o locale\lorawan-identity-print.ru\LC_MESSAGES\lorawan-identity-print.mo po\lorawan-identity-print.ru_RU.UTF-8.po

REM lorawan-identity-query.ru_RU.UTF-8.po 
xgettext -k_ -o po\tlns.pot storage/cli-query-main.cpp
msgmerge -U po\lorawan-identity-query.ru_RU.UTF-8.po  po\tlns.pot
msgfmt -o locale\lorawan-identity-query.ru\LC_MESSAGES\lorawan-identity-query.mo po\lorawan-identity-query.ru_RU.UTF-8.po

REM lorawan-identity-service.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot storage/cli-main.cpp
msgmerge -U po\lorawan-identity-service.ru_RU.UTF-8.po  po\tlns.pot
msgfmt -o locale\lorawan-identity-service.ru\LC_MESSAGES\lorawan-identity-service.mo po\lorawan-identity-service.ru_RU.UTF-8.po

REM lorawan-tag.ru_RU.UTF-8.po
xgettext -k_ -o po\tlns.pot storage/cli-tag.cpp
msgmerge -U po\lorawan-tag.ru_RU.UTF-8.po  po\tlns.pot
msgfmt -o locale\lorawan-tag.ru\LC_MESSAGES\lorawan-tag.mo po\lorawan-tag.ru_RU.UTF-8.po
