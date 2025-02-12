#ifndef LORAWAN_STORAGE_QR_HELPER_H
#define LORAWAN_STORAGE_QR_HELPER_H

#include <string>
#include <sstream>
#include <nayuki/qrcodegen.hpp>

/**
 * Returns a string of SVG code for an image depicting the given QR Code, with the given number
 * of border modules. The string always uses Unix newlines (\n), regardless of the platform.
 * @param qr
 * @param border
 * @return
 */
std::string qrCode2Svg(
    const qrcodegen::QrCode &qr,
    int border = 0
);

/**
 * Returns a string of QR code of pseudo-graphics
 * @param qr
 * @param border
 * @return
 */
std::string qrCode2Text(
    const qrcodegen::QrCode &qr,
    int border = 0
);

#endif
