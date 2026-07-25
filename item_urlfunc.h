#ifndef ITEM_URLFUNC_INCLUDED
#define ITEM_URLFUNC_INCLUDED

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

#include "item.h"

enum Url_component
{
  URL_COMPONENT_SCHEME,
  URL_COMPONENT_HOST,
  URL_COMPONENT_PATH
};

class Item_func_url_component: public Item_str_func
{
  Url_component m_component;
public:
  Item_func_url_component(THD *thd, Item *arg, Url_component component)
    : Item_str_func(thd, arg), m_component(component) {}
  String *val_str(String *) override;
  bool fix_length_and_dec(THD *) override;
  LEX_CSTRING func_name_cstring() const override;
  Item *shallow_copy(THD *thd) const override
  { return get_item_copy<Item_func_url_component>(thd, this); }
};

#endif
