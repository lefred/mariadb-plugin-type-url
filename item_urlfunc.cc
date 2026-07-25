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

#define MYSQL_SERVER
#include "mariadb.h"
#include "item_urlfunc.h"
#include "url_parser.h"

bool Item_func_url_component::fix_length_and_dec(THD *)
{
  decimals= 0;
  fix_length_and_charset(MAX_BLOB_WIDTH, default_charset());
  set_maybe_null();
  return false;
}

LEX_CSTRING Item_func_url_component::func_name_cstring() const
{
  static LEX_CSTRING names[]=
  {
    {STRING_WITH_LEN("url_scheme")},
    {STRING_WITH_LEN("url_host")},
    {STRING_WITH_LEN("url_path")}
  };
  return names[m_component];
}

String *Item_func_url_component::val_str(String *to)
{
  StringBuffer<STRING_BUFFER_USUAL_SIZE> buffer;
  String *value= args[0]->val_str(&buffer);
  Url_parts parts;
  if (!value || !url_parse(value->ptr(), value->length(), &parts))
  {
    null_value= true;
    return NULL;
  }

  const char *start;
  size_t length;
  if (m_component == URL_COMPONENT_SCHEME)
  {
    start= parts.scheme;
    length= parts.scheme_length;
  }
  else if (m_component == URL_COMPONENT_HOST)
  {
    if (!parts.host)
    {
      null_value= true;
      return NULL;
    }
    start= parts.host;
    length= parts.host_length;
  }
  else
  {
    start= parts.path;
    length= parts.path_length;
  }
  if (to->copy(start, length, value->charset()))
  {
    null_value= true;
    return NULL;
  }
  null_value= false;
  return to;
}
