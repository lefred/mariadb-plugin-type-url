/* Copyright (c) 2019,2024,2025,2026 MariaDB Corporation
   Copyright (c) 2026 lefred (Frédéric Descamps)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1335  USA */


#include "url_parser.h"
#include <assert.h>
#include <string.h>

static Url_parts parse(const char *value)
{
  Url_parts parts;
  assert(url_parse(value, strlen(value), &parts));
  return parts;
}

static void equals(const char *start, size_t length, const char *expected)
{
  assert(length == strlen(expected));
  assert(memcmp(start, expected, length) == 0);
}

int main()
{
  Url_parts p= parse("https://user:pw@example.com:8443/a/b?q=1#f");
  equals(p.scheme, p.scheme_length, "https");
  equals(p.host, p.host_length, "example.com");
  equals(p.path, p.path_length, "/a/b");

  p= parse("http://[2001:db8::1]/index.html");
  equals(p.host, p.host_length, "2001:db8::1");
  equals(p.path, p.path_length, "/index.html");

  p= parse("mailto:user@example.com");
  assert(p.host == NULL);
  equals(p.path, p.path_length, "user@example.com");

  const char *invalid[]=
  {
    "", "relative/path", "1http://example.com", "http://",
    "http://:80/", "http://example.com:/", "http://example.com:65536/",
    "http://2001:db8::1/", "http://[2001:db8::1/",
    "https://example.com/a b", "https://example.com/%xx"
  };
  for (size_t i= 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i)
    assert(!url_parse(invalid[i], strlen(invalid[i]), NULL));
  return 0;
}
