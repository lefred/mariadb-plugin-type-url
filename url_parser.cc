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

static bool ascii_alpha(unsigned char c)
{
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static bool ascii_alnum(unsigned char c)
{
  return ascii_alpha(c) || (c >= '0' && c <= '9');
}

static bool hex_digit(unsigned char c)
{
  return (c >= '0' && c <= '9') ||
         (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

static bool valid_octets(const char *s, size_t length)
{
  for (size_t i= 0; i < length; ++i)
  {
    const unsigned char c= static_cast<unsigned char>(s[i]);
    if (c <= 0x20 || c == 0x7f)
      return false;
    if (c == '%')
    {
      if (i + 2 >= length ||
          !hex_digit(static_cast<unsigned char>(s[i + 1])) ||
          !hex_digit(static_cast<unsigned char>(s[i + 2])))
        return false;
      i+= 2;
    }
  }
  return true;
}

static bool valid_port(const char *s, size_t length)
{
  if (!length)
    return false;
  unsigned long port= 0;
  for (size_t i= 0; i < length; ++i)
  {
    const unsigned char c= static_cast<unsigned char>(s[i]);
    if (c < '0' || c > '9')
      return false;
    port= port * 10 + c - '0';
    if (port > 65535)
      return false;
  }
  return true;
}

bool url_parse(const char *value, size_t length, Url_parts *parts)
{
  if (!value || !length || !ascii_alpha(static_cast<unsigned char>(value[0])) ||
      !valid_octets(value, length))
    return false;

  size_t colon= 1;
  while (colon < length && value[colon] != ':')
  {
    const unsigned char c= static_cast<unsigned char>(value[colon]);
    if (!ascii_alnum(c) && c != '+' && c != '-' && c != '.')
      return false;
    ++colon;
  }
  if (colon == length)
    return false;

  Url_parts parsed= {value, colon, NULL, 0, value + colon + 1, 0};
  size_t cursor= colon + 1;
  const bool has_authority=
    cursor + 1 < length && value[cursor] == '/' && value[cursor + 1] == '/';

  if (has_authority)
  {
    const size_t authority_start= cursor + 2;
    size_t authority_end= authority_start;
    while (authority_end < length && value[authority_end] != '/' &&
           value[authority_end] != '?' && value[authority_end] != '#')
      ++authority_end;
    if (authority_start == authority_end)
      return false;

    size_t host_start= authority_start;
    for (size_t i= authority_start; i < authority_end; ++i)
      if (value[i] == '@')
        host_start= i + 1;
    if (host_start == authority_end)
      return false;

    size_t host_end= authority_end;
    if (value[host_start] == '[')
    {
      size_t close= host_start + 1;
      while (close < authority_end && value[close] != ']')
        ++close;
      if (close == authority_end || close == host_start + 1)
        return false;
      parsed.host= value + host_start + 1;
      parsed.host_length= close - host_start - 1;
      if (close + 1 < authority_end)
      {
        if (value[close + 1] != ':' ||
            !valid_port(value + close + 2, authority_end - close - 2))
          return false;
      }
    }
    else
    {
      size_t port_separator= authority_end;
      for (size_t i= host_start; i < authority_end; ++i)
      {
        if (value[i] == ':')
        {
          if (port_separator != authority_end)
            return false; /* An IPv6 literal must use brackets. */
          port_separator= i;
        }
      }
      host_end= port_separator;
      if (host_end == host_start)
        return false;
      if (port_separator != authority_end &&
          !valid_port(value + port_separator + 1,
                      authority_end - port_separator - 1))
        return false;
      parsed.host= value + host_start;
      parsed.host_length= host_end - host_start;
    }
    cursor= authority_end;
  }

  const size_t path_start= cursor;
  while (cursor < length && value[cursor] != '?' && value[cursor] != '#')
    ++cursor;
  parsed.path= value + path_start;
  parsed.path_length= cursor - path_start;

  /* An absolute URI without authority must have a non-empty path. */
  if (!has_authority && parsed.path_length == 0)
    return false;

  if (parts)
    *parts= parsed;
  return true;
}
