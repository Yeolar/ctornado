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

#define LOOP    100000000

inline void test_c_str()
{
    const char *s1;
    char *s2, *s3;
    size_t len;

    s1 = "a very long literal string";
    len = strlen(s1) + 1;

    for (int i = 0; i != LOOP; ++i) {
        s2 = new char[len + 1];
        strcpy(s2, s1);

        s3 = new char[4];
        strncpy(s3, s2 + 2, 3);
        s3[3] = '\0';

        delete[] s2;
        delete[] s3;
    }
}

inline void test_cpp_str()
{
    string s1("a very long literal string");
    string s2, s3;

    for (int i = 0; i != LOOP; ++i) {
        s2 = s1;
        s3 = s2.substr(2, 5);
    }
}

inline void test_my_str()
{
    Str s1("a very long literal string");
    Str s2, s3;

    for (int i = 0; i != LOOP; ++i) {
        s2 = s1;
        s3 = s2.substr(2, 5);
    }
}

int main()
{
    Logger::initialize(Logger::INFO);

    // test eq
    log_stderr("test eq:        abcde %s abdce",
            Str("abcde").eq("abdce") ? "==" : "!=");
    log_stderr("test eq:        abcde %s abcde",
            Str("abcde").eq("abcde") ? "==" : "!=");

    // test find
    log_stderr("test find:      abcdefgabcdefg find d at %d",
            Str("abcdefgabcdefg").find('d'));
    log_stderr("test find:      abcdefgabcdefg find def at %d",
            Str("abcdefgabcdefg").find("def"));

    // test rfind
    log_stderr("test rfind:     abcdefgabcdefg rfind d at %d",
            Str("abcdefgabcdefg").rfind('d'));
    log_stderr("test rfind:     abcdefgabcdefg rfind def at %d",
            Str("abcdefgabcdefg").rfind("def"));

    // test count
    log_stderr("test count:     abcdefgabcdefg count d at %d",
            Str("abcdefgabcdefg").count("d"));
    log_stderr("test count:     abcdefgabcdefg count def at %d",
            Str("abcdefgabcdefg").count("def"));

    // test lstrip, rstrip, strip
    log_stderr("test lstrip:    ' abcdefgabcdefg  ' lstrip: '%s'",
            Str(" abcdefgabcdefg  ").lstrip().tos().c_str());
    log_stderr("test rstrip:    ' abcdefgabcdefg  ' rstrip: '%s'",
            Str(" abcdefgabcdefg  ").rstrip().tos().c_str());
    log_stderr("test strip:     ' abcdefgabcdefg  ' strip: '%s'",
            Str(" abcdefgabcdefg  ").strip().tos().c_str());

    // test capitalize, capitalize_each
    log_stderr("test capitalize: abcdefg-abcdefg capitalize: %s",
            Str("abcdefg-abcdefg").capitalize().tos().c_str());
    log_stderr("test capitalize_each: abcdefg-abcdefg capitalize_each: %s",
            Str("abcdefg-abcdefg").capitalize_each('-').tos().c_str());

    // test escape
    log_stderr("test escape:    abc\\rde\\nf\\r\\ng\\t escape: %s",
            Str("abc\rde\nf\r\ng\t").escape().tos().c_str());

    // test split_lines
    log_stderr("test split_lines: "
            "\\r\\nabcdefg\\r\\nabcdef\\rabcde\\nabcd split:");

    for (auto& s : Str("\r\nabcdefg\r\nabcdef\rabcde\nabcd").split_lines()) {
        log_stderr(s.tos().c_str());
    }

    // test replace
    log_stderr("test replace:   abcdefgabcdefg replace ab to xyz: %s",
            Str("abcdefgabcdefg").replace("ab", "xyz").tos().c_str());

    //
    // test performance
    //
    log_stderr("test performance:");

    ClockTimer timer;

    timer.start();
    test_c_str();
    timer.stop();
    log_stderr("C strings:   %f seconds.", timer.seconds());

    timer.start();
    test_cpp_str();
    timer.stop();
    log_stderr("C++ strings: %f seconds.", timer.seconds());

    timer.start();
    test_my_str();
    timer.stop();
    log_stderr("My strings:  %f seconds.", timer.seconds());

    return 0;
}
