//
// Copyright (C) 2013 Yeolar <yeolar@gmail.com>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

#include "ctornado.h"

using namespace ctornado;

int main()
{
    const char *raw =
        "Test GZip IO, a C++ wrapper of gzip compression, written by Yeolar";
    GZipCompressor *gzip_compressor;
    GZipDecompressor *gzip_decompressor;
    Str compressed, decompressed;

    Logger::initialize(Logger::INFO);

    log_stderr("Raw string:\n%s", raw);

    gzip_compressor = new GZipCompressor();
    gzip_compressor->compress(raw);
    gzip_compressor->close();

    compressed = gzip_compressor->get_value();
    log_stderr("GZip compress =>\n%s", hexlify(compressed).tos().c_str());

    delete gzip_compressor;

    gzip_decompressor = new GZipDecompressor();
    gzip_decompressor->decompress(compressed);
    gzip_decompressor->close();

    decompressed = gzip_decompressor->get_value();
    log_stderr("GZip decompress =>\n%s", decompressed.tos().c_str());

    delete gzip_decompressor;

    return 0;
}

