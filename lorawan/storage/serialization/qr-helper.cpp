#include "lorawan/storage/serialization/qr-helper.h"

/**
 * Returns a string of SVG code for an image depicting the given QR Code, with the given number
 * of border modules. The string always uses Unix newlines (\n), regardless of the platform.
 * @param qr
 * @param border
 * @return
 */
std::string qrCode2Svg(
    const qrcodegen::QrCode &qr,
    int border
) {
    if (border < 0)
        border = 1;
    auto sz = qr.getSize();
    if (border > INT32_MAX / 2 || border * 2 > INT32_MAX - sz)
        border = 1;
    std::ostringstream sb;
    sb << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    sb << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\" \"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n";
    sb << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" viewBox=\"0 0 ";
    sb << (sz + border * 2) << " " << (sz + border * 2) << "\" stroke=\"none\">\n";
    sb << "\t<rect width=\"100%\" height=\"100%\" fill=\"#FFFFFF\"/>\n";
    sb << "\t<path d=\"";
    for (int y = 0; y < sz; y++) {
        for (int x = 0; x < sz; x++) {
            if (qr.getModule(x, y)) {
                if (x != 0 || y != 0)
                    sb << " ";
                sb << "M" << (x + border) << "," << (y + border) << "h1v1h-1z";
            }
        }
    }
    sb << "\" fill=\"#000000\"/>\n";
    sb << "</svg>\n";
    return sb.str();
}

/**
 * Returns a string of QR code of pseudo-graphics
 * @param qr
 * @param border
 * @return
 */
std::string qrCode2Text(
    const qrcodegen::QrCode &qr,
    int border
) {
    std::stringstream ss;
    for (int y = -border; y < qr.getSize() + border; y++) {
        for (int x = -border; x < qr.getSize() + border; x++) {
            ss << (qr.getModule(x, y) ? "  " : u8"\u2588\u2588");
        }
        ss << "\n";
    }
    ss << "\n";
    return ss.str();
}
