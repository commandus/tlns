#!/bin/sh
# qr - image
# prop - with proprietary
# text - print text otherwise SVG
wget -O - --post-data "LW:D0:::PXA01450340" http://127.0.0.1:4248/qr-prop-text
# wget -O - --post-data "LW:D0:::PXA01450340" http://127.0.0.1:4248/prop