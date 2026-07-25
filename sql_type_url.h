#ifndef SQL_TYPE_URL_INCLUDED
#define SQL_TYPE_URL_INCLUDED

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

#include "sql_type.h"
#include "field.h"

class Type_handler_url: public Type_handler_long_blob
{
public:
  static constexpr LEX_CSTRING name_on_client{STRING_WITH_LEN("url")};
  const Type_collection *type_collection() const override;
  uint get_column_attributes() const override { return ATTR_CHARSET; }
  const Type_handler *type_handler_for_comparison() const override;
  Item *create_typecast_item(THD *, Item *,
                             const Type_cast_attributes &) const override;
  Field *make_conversion_table_field(MEM_ROOT *, TABLE *, uint,
                                     const Field *) const override;
  Log_event_data_type user_var_log_event_data_type(uint charset_nr)
                                                        const override;
  bool Column_definition_prepare_stage1(
    THD *, MEM_ROOT *, Column_definition *, column_definition_type_t,
    const Column_derived_attributes *) const override;
  Field *make_table_field(MEM_ROOT *, const LEX_CSTRING *,
                          const Record_addr &, const Type_all_attributes &,
                          TABLE_SHARE *) const override;
  Field *make_table_field_from_def(TABLE_SHARE *, MEM_ROOT *,
    const LEX_CSTRING *, const Record_addr &, const Bit_addr &,
    const Column_definition_attributes *, uint32) const override;
  Item *make_constructor_item(THD *, List<Item> *) const override;
  bool Item_hybrid_func_fix_attributes(
    THD *, const LEX_CSTRING &, Type_handler_hybrid_field_type *,
    Type_all_attributes *, Item **, uint) const override;
  const Type_handler *type_handler_for_tmp_table(const Item *) const override;
  bool Item_append_extended_type_info(Send_field_extended_metadata *to,
                                      const Item *) const override
  { return to->set_data_type_name(name_on_client); }
  bool can_return_int() const override { return false; }
  bool can_return_decimal() const override { return false; }
  bool can_return_real() const override { return false; }
  bool can_return_date() const override { return false; }
  bool can_return_time() const override { return false; }
};

extern Type_handler_url type_handler_url;

class Type_collection_url: public Type_collection
{
public:
  const Type_handler *aggregate_for_result(const Type_handler *,
                                           const Type_handler *) const override;
  const Type_handler *aggregate_for_comparison(
    const Type_handler *, const Type_handler *) const override;
  const Type_handler *aggregate_for_min_max(const Type_handler *,
                                            const Type_handler *) const override;
  const Type_handler *aggregate_for_num_op(const Type_handler *,
                                           const Type_handler *) const override;
};

class Field_url: public Field_blob
{
public:
  Field_url(uchar *ptr, uchar *null_ptr, uchar null_bit, enum utype unireg,
            const LEX_CSTRING *name, TABLE_SHARE *share,
            const DTCollation &collation)
    : Field_blob(ptr, null_ptr, null_bit, unireg, name, share, 4, collation) {}
  const Type_handler *type_handler() const override { return &type_handler_url; }
  void make_send_field(Send_field *to) override
  {
    Field_longstr::make_send_field(to);
    to->set_data_type_name(Type_handler_url::name_on_client);
  }
  void sql_type(String &) const override;
  uint size_of() const override { return sizeof(*this); }
  int store(const char *, size_t, CHARSET_INFO *) override;
  using Field_str::store;
  enum_conv_type rpl_conv_type_from(const Conv_source &,
    const Relay_log_info *, const Conv_param &) const override;
};

class Item_url_typecast: public Item_char_typecast
{
public:
  Item_url_typecast(THD *thd, Item *arg, CHARSET_INFO *cs)
    : Item_char_typecast(thd, arg, -1, cs) {}
  const Type_handler *type_handler() const override { return &type_handler_url; }
  LEX_CSTRING func_name_cstring() const override;
  bool fix_length_and_dec(THD *) override;
  String *val_str(String *) override;
  Item *shallow_copy(THD *thd) const override
  { return get_item_copy<Item_url_typecast>(thd, this); }
  void print(String *, enum_query_type) override;
};

#endif
