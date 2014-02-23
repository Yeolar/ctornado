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

/* http://www.w3.org/TR/html401/interact/forms.html#h-17.13.4.2

The following example illustrates "multipart/form-data" encoding. Suppose we have the following form:

 <FORM action="http://server.com/cgi/handle"
       enctype="multipart/form-data"
       method="post">
   <P>
   What is your name? <INPUT type="text" name="submit-name"><BR>
   What files are you sending? <INPUT type="file" name="files"><BR>
   <INPUT type="submit" value="Send"> <INPUT type="reset">
 </FORM>

If the user enters "Larry" in the text input, and selects the text file "file1.txt", the user agent might send back the following data:

   Content-Type: multipart/form-data; boundary=AaB03x

   --AaB03x
   Content-Disposition: form-data; name="submit-name"

   Larry
   --AaB03x
   Content-Disposition: form-data; name="files"; filename="file1.txt"
   Content-Type: text/plain

   ... contents of file1.txt ...
   --AaB03x--

If the user selected a second (image) file "file2.gif", the user agent might construct the parts as follows:

   Content-Type: multipart/form-data; boundary=AaB03x

   --AaB03x
   Content-Disposition: form-data; name="submit-name"

   Larry
   --AaB03x
   Content-Disposition: form-data; name="files"
   Content-Type: multipart/mixed; boundary=BbC04y

   --BbC04y
   Content-Disposition: file; filename="file1.txt"
   Content-Type: text/plain

   ... contents of file1.txt ...
   --BbC04y
   Content-Disposition: file; filename="file2.gif"
   Content-Type: image/gif
   Content-Transfer-Encoding: binary

   ...contents of file2.gif...
   --BbC04y--
   --AaB03x--
 */

int main()
{
    const char *header =
    "Accept:text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
    "Accept-Encoding:gzip,deflate,sdch\r\n"
    "Accept-Language:zh-CN,zh;q=0.8,en;q=0.6\r\n"
    "Cache-Control:max-age=0\r\n"
    "Connection:keep-alive\r\n"
    "Cookie:csrftoken=20fe000000000000000000000000d2e9\r\n"
    "Host:www.yeolar.com\r\n"
    "User-Agent:Mozilla/5.0 (Macintosh; Intel Mac OS X 10_7_5) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/31.0.1650.63 Safari/537.36";
    const char *content_type = "multipart/form-data; boundary=AaB03x";
    const char *body =
    "--AaB03x\r\n"
    "Content-Disposition: form-data; name=\"submit-name\"\r\n"
    "\r\n"
    "Larry\r\n"
    "--AaB03x\r\n"
    "Content-Disposition: form-data; name=\"files\"; filename=\"file1.txt\"\r\n"
    "Content-Type: text/plain\r\n"
    "\r\n"
    "... contents of file1.txt ...\r\n"
    "--AaB03x--";
    HTTPHeaders *headers;
    Query *q;
    FileMMap *m;
    HTTPFile *f;

    Logger::initialize(Logger::VERB);

    log_stderr("Parse headers:\n\n%s\n", header);
    headers = HTTPHeaders::parse(header);

    log_stderr("Header has key 'User-Agent'?    %s",
            headers->has("User-Agent") ? "yes" : "no");
    log_stderr("Header key 'host':              %s",
            headers->get("host").tos().c_str());
    log_stderr("Normalize key 'cache-control':  %s",
            headers->normalize_name("cache-control").tos().c_str());
    log_stderr("");

    q = new Query();
    m = new FileMMap();

    log_stderr("Parse body arguments:\n\n%s\n", body);
    parse_body_arguments(content_type, body, q, m);

    log_stderr("Argument submit-name: %s", q->get("submit-name").tos().c_str());
    log_stderr("Files:");

    for (auto& kv : *m) {
        f = &kv.second;
        log_stderr("  name:         %s", kv.first.tos().c_str());
        log_stderr("  filename:     %s", f->filename_.tos().c_str());
        log_stderr("  body:         %s", f->body_.tos().c_str());
        log_stderr("  content-type: %s", f->content_type_.tos().c_str());
    }

    return 0;
}

