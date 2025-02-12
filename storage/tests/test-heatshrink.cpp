#include <string>
#include <iostream>
#include <cassert>
#include "lorawan/storage/service/identity-service-json.h"

extern "C" {
#include "heatshrink/heatshrink_encoder.h"
#include "heatshrink/heatshrink_decoder.h"
}

#if HEATSHRINK_DYNAMIC_ALLOC
#error HEATSHRINK_DYNAMIC_ALLOC must be false for static allocation test suite.
#endif

static std::string s1("# heatshrink\n"
                         "\n"
                         "A data compression/decompression library for embedded/real-time systems.\n"
                         "\n"
                         "\n"
                         "## Key Features:\n"
                         "\n"
                         "- **Low memory usage (as low as 50 bytes)**\n"
                         "    It is useful for some cases with less than 50 bytes, and useful\n"
                         "    for many general cases with < 300 bytes.\n"
                         "- **Incremental, bounded CPU use**\n"
                         "    You can chew on input data in arbitrarily tiny bites.\n"
                         "    This is a useful property in hard real-time environments.\n"
                         "- **Can use either static or dynamic memory allocation**\n"
                         "    The library doesn't impose any constraints on memory management.\n"
                         "- **ISC license**\n"
                         "    You can use it freely, even for commercial purposes.\n"
                         "\n"
                         "\n"
                         "## Getting Started:\n"
                         "\n"
                         "There is a standalone command-line program, `heatshrink`, but the\n"
                         "encoder and decoder can also be used as libraries, independent of each\n"
                         "other. To do so, copy `heatshrink_common.h`, `heatshrink_config.h`, and\n"
                         "either `heatshrink_encoder.c` or `heatshrink_decoder.c` (and their\n"
                         "respective header) into your project. For projects that use both,\n"
                         "static libraries are built that use static and dynamic allocation.\n"
                         "\n"
                         "Dynamic allocation is used by default, but in an embedded context, you\n"
                         "probably want to statically allocate the encoder/decoder. Set\n"
                         "`HEATSHRINK_DYNAMIC_ALLOC` to 0 in `heatshrink_config.h`.\n"
                         "\n"
                         "\n"
                         "### Basic Usage\n"
                         "\n"
                         "1. Allocate a `heatshrink_encoder` or `heatshrink_decoder` state machine\n"
                         "using their `alloc` function, or statically allocate one and call their\n"
                         "`reset` function to initialize them. (See below for configuration\n"
                         "options.)\n"
                         "\n"
                         "2. Use `sink` to sink an input buffer into the state machine. The\n"
                         "`input_size` pointer argument will be set to indicate how many bytes of\n"
                         "the input buffer were actually consumed. (If 0 bytes were conusmed, the\n"
                         "buffer is full.)\n"
                         "\n"
                         "3. Use `poll` to move output from the state machine into an output\n"
                         "buffer. The `output_size` pointer argument will be set to indicate how\n"
                         "many bytes were output, and the function return value will indicate\n"
                         "whether further output is available. (The state machine may not output\n"
                         "any data until it has received enough input.)\n"
                         "\n"
                         "Repeat steps 2 and 3 to stream data through the state machine. Since\n"
                         "it's doing data compression, the input and output sizes can vary\n"
                         "significantly. Looping will be necessary to buffer the input and output\n"
                         "as the data is processed.\n"
                         "\n"
                         "4. When the end of the input stream is reached, call `finish` to notify\n"
                         "the state machine that no more input is available. The return value from\n"
                         "`finish` will indicate whether any output remains. if so, call `poll` to\n"
                         "get more.\n"
                         "\n"
                         "Continue calling `finish` and `poll`ing to flush remaining output until\n"
                         "`finish` indicates that the output has been exhausted.\n"
                         "\n"
                         "Sinking more data after `finish` has been called will not work without\n"
                         "calling `reset` on the state machine.\n"
                         "\n"
                         "\n"
                         "## Configuration\n"
                         "\n"
                         "heatshrink has a couple configuration options, which impact its resource\n"
                         "usage and how effectively it can compress data. These are set when\n"
                         "dynamically allocating an encoder or decoder, or in `heatshrink_config.h`\n"
                         "if they are statically allocated.\n"
                         "\n"
                         "- `window_sz2`, `-w` in the CLI: Set the window size to 2^W bytes.\n"
                         "\n"
                         "The window size determines how far back in the input can be searched for\n"
                         "repeated patterns. A `window_sz2` of 8 will only use 256 bytes (2^8),\n"
                         "while a `window_sz2` of 10 will use 1024 bytes (2^10). The latter uses\n"
                         "more memory, but may also compress more effectively by detecting more\n"
                         "repetition.\n"
                         "\n"
                         "The `window_sz2` setting currently must be between 4 and 15.\n"
                         "\n"
                         "- `lookahead_sz2`, `-l` in the CLI: Set the lookahead size to 2^L bytes.\n"
                         "\n"
                         "The lookahead size determines the max length for repeated patterns that\n"
                         "are found. If the `lookahead_sz2` is 4, a 50-byte run of 'a' characters\n"
                         "will be represented as several repeated 16-byte patterns (2^4 is 16),\n"
                         "whereas a larger `lookahead_sz2` may be able to represent it all at\n"
                         "once. The number of bits used for the lookahead size is fixed, so an\n"
                         "overly large lookahead size can reduce compression by adding unused\n"
                         "size bits to small patterns.\n"
                         "\n"
                         "The `lookahead_sz2` setting currently must be between 3 and the\n"
                         "`window_sz2` - 1.\n"
                         "\n"
                         "- `input_buffer_size` - How large an input buffer to use for the\n"
                         "decoder. This impacts how much work the decoder can do in a single\n"
                         "step, and a larger buffer will use more memory. An extremely small\n"
                         "buffer (say, 1 byte) will add overhead due to lots of suspend/resume\n"
                         "function calls, but should not change how well data compresses.\n"
                         "\n"
                         "\n"
                         "### Recommended Defaults\n"
                         "\n"
                         "For embedded/low memory contexts, a `window_sz2` in the 8 to 10 range is\n"
                         "probably a good default, depending on how tight memory is. Smaller or\n"
                         "larger window sizes may make better trade-offs in specific\n"
                         "circumstances, but should be checked with representative data.\n"
                         "\n"
                         "The `lookahead_sz2` should probably start near the `window_sz2`/2, e.g.\n"
                         "-w 8 -l 4 or -w 10 -l 5. The command-line program can be used to measure\n"
                         "how well test data works with different settings.\n"
                         "\n"
                         "\n"
                         "## More Information and Benchmarks:\n"
                         "\n"
                         "heatshrink is based on [LZSS], since it's particularly suitable for\n"
                         "compression in small amounts of memory. It can use an optional, small\n"
                         "[index] to make compression significantly faster, but otherwise can run\n"
                         "in under 100 bytes of memory. The index currently adds 2^(window size+1)\n"
                         "bytes to memory usage for compression, and temporarily allocates 512\n"
                         "bytes on the stack during index construction (if the index is enabled).\n"
                         "\n"
                         "For more information, see the [blog post] for an overview, and the\n"
                         "`heatshrink_encoder.h` / `heatshrink_decoder.h` header files for API\n"
                         "documentation.\n"
                         "\n"
                         "[blog post]: http://spin.atomicobject.com/2013/03/14/heatshrink-embedded-data-compression/\n"
                         "[index]: http://spin.atomicobject.com/2014/01/13/lightweight-indexing-for-embedded-systems/\n"
                         "[LZSS]: http://en.wikipedia.org/wiki/Lempel-Ziv-Storer-Szymanski\n"
                         "\n"
                         "\n"
                         "## Build Status\n"
                         "\n"
                         "  [![Build Status](https://travis-ci.org/atomicobject/heatshrink.png)](http://travis-ci.org/atomicobject/heatshrink)");

static std::string s2("1234567890abcdefghijklmnopqrtst");

static size_t compressBuffer(
    void *output,
    size_t outputSize,
    const void *input,
    size_t inputSize
) {
    heatshrink_encoder hse;
    heatshrink_encoder_reset(&hse);
    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;
    uint8_t *pOut = (uint8_t *) output;
    uint8_t *pIn = (uint8_t *) input;
    while (sunk < inputSize) {
        if (heatshrink_encoder_sink(&hse, pIn, inputSize - sunk, &count) < 0)
            return 0;
        pIn += count;
        sunk += count;
        if (sunk == inputSize)
            heatshrink_encoder_finish(&hse);
        HSE_poll_res pr;
        do {
            pr = heatshrink_encoder_poll(&hse, pOut, outputSize - polled, &count);
            if (pr < 0)
                return 0;
            polled += count;
            pOut += count;
        } while (pr == HSER_POLL_MORE);
        if (polled >= outputSize)
            return 0;
        if (sunk == inputSize)
            heatshrink_encoder_finish(&hse);
    }
    return polled;
}

static size_t decompressBuffer(
    void *output,
    size_t outputSize,
    const void  *input,
    size_t inputSize
) {
    heatshrink_decoder hsd;
    heatshrink_decoder_reset(&hsd);
    size_t count = 0;
    size_t sunk = 0;
    size_t polled = 0;

    uint8_t *pOut = (uint8_t *) output;
    uint8_t *pIn = (uint8_t *) input;
    while (sunk < inputSize) {
        auto h = heatshrink_decoder_sink(&hsd, pIn, inputSize - sunk, &count);
        if (h == HSDR_SINK_FULL)
            return polled;
        sunk += count;
        pIn += count;
        HSD_poll_res pr;
        if (sunk == inputSize)
            heatshrink_decoder_finish(&hsd);
        do {
            pr = heatshrink_decoder_poll(&hsd, pOut, outputSize - polled, &count);
            polled += count;
            pOut += count;
        } while (pr == HSDR_POLL_MORE && count > 0);
        if (polled > outputSize)
            return 0;
    }
    return polled;
}

static void testText(
    const std::string &s
)
{
    size_t compressedBufferSize = 2 * s.size();
    char *compressedBuffer = (char *) malloc(compressedBufferSize);
    memset(compressedBuffer, 0, compressedBufferSize);
    size_t compressedSize = compressBuffer(compressedBuffer, compressedBufferSize, s.c_str(), s.size());

    std::cout << s.size() << "->" << compressedSize << std::endl;

    size_t decompressedBufferSize = s.size();
    char *decompressedBuffer = (char *) malloc(decompressedBufferSize);
    memset(decompressedBuffer, 0, decompressedBufferSize);
    size_t decompressedSize = decompressBuffer(decompressedBuffer, decompressedBufferSize, compressedBuffer, compressedSize);
    std::cout << compressedSize << "->" << decompressedSize << "(" << decompressedBufferSize << ")" << std::endl;

    assert(decompressedSize == s.size());

    free(decompressedBuffer);
    free(compressedBuffer);
}

static void testIdentities()
{
    JsonIdentityService identityService;
    identityService.init("test.json", nullptr);

    std::vector<NETWORKIDENTITY> nis;
    identityService.list(nis, 0, 255);
    char compressedBuffer[sizeof(DEVICE_ID)];
    for (auto &n : nis) {
        size_t compressedSize = compressBuffer(compressedBuffer, sizeof(compressedBuffer), &n.value.devid.id, sizeof(DEVICE_ID));

        DEVICE_ID decompressedDid;
        size_t decompressedSize = decompressBuffer(&decompressedDid, sizeof(decompressedDid), compressedBuffer, compressedSize);

        std::cout << sizeof(DEVICE_ID) << "->" << compressedSize << std::endl;
    }
}

static void testIdentities2()
{
    JsonIdentityService identityService;
    identityService.init("test.json", nullptr);

    std::vector<NETWORKIDENTITY> nis;
    identityService.list(nis, 0, 255);
    DEVICE_ID compressedBuffer;
    for (auto &n : nis) {
        size_t compressedSize = compressBuffer(&compressedBuffer, sizeof(compressedBuffer), &n.value.devid.id, sizeof(DEVICE_ID));
        DEVICE_ID decompressedDid;
        size_t decompressedSize = decompressBuffer(&decompressedDid, sizeof(decompressedDid), &compressedBuffer, compressedSize);
        std::cout << sizeof(DEVICE_ID) << "->" << compressedSize << std::endl;

        // decompressedSize = decompressBuffer(&decompressedDid, sizeof(decompressedDid), &n.value.devid.id, sizeof(DEVICE_ID));
        // std::cout << "Decompress uncompressed " << sizeof(DEVICE_ID) << "->" << decompressedSize << std::endl;

        // DEVICE_ID b;
        // size_t ccompressedSize = compressBuffer(&b, sizeof(DEVICE_ID), &compressedBuffer, compressedSize);
        // std::cout << "Compress compressed " << compressedSize << "->" << ccompressedSize << std::endl;
    }
}

int main() {
    testText(s1);
    testText(s2);
    testIdentities();
    testIdentities2();
    return 0;
}
