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

namespace ctornado {

Regex *HTTPHeaders::normalized_header_re_ = Regex::compile(
        "^[A-Z0-9][a-z0-9]*(-[A-Z0-9][a-z0-9]*)*$");

ss_map_t HTTPHeaders::normalized_headers_;

HTTPHeaders *HTTPHeaders::parse(const Str& str)
{
    HTTPHeaders *headers;
    s_list_t lines;
    Str line;

    headers = new HTTPHeaders();
    lines = str.split_lines();

    for (auto it = lines.begin(); it != lines.end(); ) {
        line = *it;
        it++;

        while (it != lines.end() && isspace((*it)[0])) {
            line = line.concat(*it);
            it++;
        }

        auto pair = line.split_pair(':');
        if (!pair.first.null() && !pair.second.null()) {
            headers->add(pair.first, pair.second.strip());
        }
    }
    return headers;
}

bool HTTPHeaders::has(const Str& name)
{
    return map_.find(normalize_name(name)) != map_.end();
}

void HTTPHeaders::add(const Str& name, const Str& value)
{
    Str norm_name;

    norm_name = normalize_name(name);

    if (map_.find(norm_name) != map_.end()) {
        map_[norm_name] = Str::sprintf("%S,%S", &map_[norm_name], &value);
    }
    else {
        map_[norm_name] = value;
    }
    log_verb("headers add (%.*s, %.*s)",
            norm_name.len_, norm_name.data_,
            map_[norm_name].len_, map_[norm_name].data_);
}

Str HTTPHeaders::get(const Str& name, const Str& deft)
{
    try {
        return map_.at(normalize_name(name));
    }
    catch (std::out_of_range) {
        return deft;
    }
}

ss_map_t *HTTPHeaders::get_all()
{
    return &map_;
}

Str HTTPHeaders::normalize_name(const Str& name)
{
    Str normalized;

    try {
        return normalized_headers_.at(name);
    }
    catch (std::out_of_range) {
        RegexMatch m = normalized_header_re_->exec(name, 1);

        if (!m.empty())
            normalized = name;
        else
            normalized = name.capitalize_each('-');

        normalized_headers_[name] = normalized;
        return normalized;
    }
}

static int _parse_param(const Str& str)
{
    int end;

    end = str.find(';');

    while (end > 0 &&
            ((str.count('"', 0, end) - str.count("\\\"", 0, end)) % 2)) {
        end = str.find(';', end + 1);
    }
    if (end < 0) {
        end = str.len_;
    }
    return end;
}

//
// Parse a Content-type like header.
//
// Return the main content-type and a dictionary of options.
//
static Str _parse_header(const Str& line, ss_map_t *pdict)
{
    int pos, i;
    Str key, parts, p, name, value;

    pos = _parse_param(line);
    key = line.substr(0, pos);
    parts = line;

    while (pos < parts.len_ && parts[pos] == ';') {
        parts = parts.substr(pos + 1, -1);
        pos = _parse_param(parts);
        p = parts.substr(0, pos);

        i = p.find('=');
        if (i >= 0) {
            name = p.substr(0, i).strip().lower();
            value = p.substr(i + 1, -1).strip();

            if (value.len_ >= 2 && value[0] == '"' && value[-1] == '"') {
                value = value.substr(1, value.len_ - 1);
                value = value.replace("\\\\", "\\").replace("\\\"", "\"");
            }
            pdict->insert({ name, value });
        }
    }
    return key;
}

void parse_body_arguments(const Str& content_type, const Str& body,
        Query *arguments, file_mmap_t *files)
{
    bool found;

    if (content_type.starts_with("application/x-www-form-urlencoded")) {
        arguments->parse_extend(body);
    }
    else if (content_type.starts_with("multipart/form-data")) {
        found = false;

        for (auto& field : content_type.split(';')) {
            auto kv = field.strip().split_pair('=');

            if (kv.first.eq("boundary") && !kv.second.null()) {
                parse_multipart_form_data(kv.second, body, arguments, files);
                found = true;
                break;
            }
        }
        if (!found)
            log_warn("Invalid multipart/form-data");
    }
}

void parse_multipart_form_data(const Str& boundary, const Str& data,
        Query *arguments, file_mmap_t *files)
{
    Str bound;
    int final_boundary_index, eoh;
    HTTPHeaders *headers;
    ss_map_t disp_params;

    //
    // The standard allows for the boundary to be quoted in the header,
    // although it's rare (it happens at least for google app engine
    // xmpp).  I think we're also supposed to handle backslash-escapes
    // here but I'll save that until we see a client that uses them
    // in the wild.
    //
    if (boundary[0] == '"' && boundary[-1] == '"') {
        bound = boundary.substr(1, boundary.len_ - 1);
    }
    else {
        bound = boundary;
    }

    final_boundary_index = data.rfind(Str::sprintf("--%S--", &bound));
    if (final_boundary_index == -1) {
        log_warn("Invalid multipart/form-data: no final boundary");
        return;
    }

    auto parts = data.substr(0, final_boundary_index).split(
            Str::sprintf("--%S\r\n", &bound));

    for (auto& part : parts) {
        if (part.null())
            continue;

        eoh = part.find("\r\n\r\n");
        if (eoh == -1) {
            log_warn("multipart/form-data missing headers");
            continue;
        }

        headers = HTTPHeaders::parse(part.substr(0, eoh));
        Str disp_header = headers->get("Content-Disposition", "");
        Str disposition = _parse_header(disp_header, &disp_params);

        if (!disposition.eq("form-data") || !part.ends_with("\r\n")) {
            log_warn("Invalid multipart/form-data");
            delete headers;
            continue;
        }
        Str value = part.substr(eoh + 4, part.len_ - 2);

        auto it = disp_params.find("name");
        if (it == disp_params.end()) {
            log_warn("multipart/form-data value missing name");
            delete headers;
            continue;
        }
        Str name = it->second;

        it  = disp_params.find("filename");
        if (it == disp_params.end()) {
            arguments->add(name, value);
        }
        else {
            Str ctype = headers->get("Content-Type", "application/unknown");
            files->insert({ name, HTTPFile(it->second, value, ctype) });
        }
        delete headers;

        disp_params.clear();
    }
}

} // namespace
