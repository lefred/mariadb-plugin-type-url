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
#include "sql_class.h"
#include "sql_lex.h"
#include "sql_type_url.h"
#include "url_parser.h"

Type_handler_url type_handler_url;
static Type_collection_url type_collection_url;

constexpr LEX_CSTRING Type_handler_url::name_on_client;

const Type_collection *Type_handler_url::type_collection() const
{
  return &type_collection_url;
}

static const Type_handler *aggregate_url(const Type_handler *a,
                                         const Type_handler *b)
{
  if (a == &type_handler_url)
    swap_variables(const Type_handler *, a, b);
  if (a == &type_handler_url || a == &type_handler_hex_hybrid ||
      a == &type_handler_tiny_blob || a == &type_handler_blob ||
      a == &type_handler_medium_blob || a == &type_handler_long_blob ||
      a == &type_handler_varchar || a == &type_handler_string ||
      a == &type_handler_null)
    return b;
  return NULL;
}

const Type_handler *Type_collection_url::aggregate_for_comparison(
  const Type_handler *a, const Type_handler *b) const
{ return aggregate_url(a, b); }

const Type_handler *Type_collection_url::aggregate_for_result(
  const Type_handler *a, const Type_handler *b) const
{ return aggregate_url(a, b); }

const Type_handler *Type_collection_url::aggregate_for_min_max(
  const Type_handler *a, const Type_handler *b) const
{ return aggregate_url(a, b); }

const Type_handler *Type_collection_url::aggregate_for_num_op(
  const Type_handler *, const Type_handler *) const
{ return NULL; }

const Type_handler *Type_handler_url::type_handler_for_comparison() const
{ return &type_handler_url; }

Log_event_data_type
Type_handler_url::user_var_log_event_data_type(uint charset_nr) const
{
  return Log_event_data_type(name().lex_cstring(), result_type(),
                             charset_nr, false);
}

Field *Type_handler_url::make_conversion_table_field(
  MEM_ROOT *root, TABLE *table, uint metadata, const Field *target) const
{
  if ((metadata & 0xff) != 4)
    return NULL;
  return new (root) Field_url(NULL, (uchar *) "", 1, Field::NONE,
                              &empty_clex_str, table->s, target->charset());
}

Item *Type_handler_url::create_typecast_item(
  THD *thd, Item *item, const Type_cast_attributes &attr) const
{
  CHARSET_INFO *cs= attr.charset() ? attr.charset() :
                                     thd->variables.collation_connection;
  if (cs == &my_charset_bin)
  {
    my_error(ER_ILLEGAL_PARAMETER_DATA_TYPE_FOR_OPERATION, MYF(0),
             name().ptr(), "CHARACTER SET binary");
    return NULL;
  }
  return new (thd->mem_root) Item_url_typecast(thd, item, cs);
}

bool Type_handler_url::Column_definition_prepare_stage1(
  THD *thd, MEM_ROOT *root, Column_definition *def,
  column_definition_type_t type, const Column_derived_attributes *derived) const
{
  if (Type_handler_long_blob::Column_definition_prepare_stage1(
        thd, root, def, type, derived))
    return true;
  if (def->charset == &my_charset_bin)
  {
    my_error(ER_ILLEGAL_PARAMETER_DATA_TYPE_FOR_OPERATION, MYF(0),
             name().ptr(), "CHARACTER SET binary");
    return true;
  }
  return false;
}

Field *Type_handler_url::make_table_field(
  MEM_ROOT *root, const LEX_CSTRING *name, const Record_addr &addr,
  const Type_all_attributes &attr, TABLE_SHARE *share) const
{
  return new (root) Field_url(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                              Field::NONE, name, share, attr.collation);
}

Field *Type_handler_url::make_table_field_from_def(
  TABLE_SHARE *share, MEM_ROOT *root, const LEX_CSTRING *name,
  const Record_addr &addr, const Bit_addr &,
  const Column_definition_attributes *attr, uint32) const
{
  return new (root) Field_url(addr.ptr(), addr.null_ptr(), addr.null_bit(),
                              attr->unireg_check, name, share, attr->charset);
}

Item *Type_handler_url::make_constructor_item(THD *thd, List<Item> *args) const
{
  if (!args || args->elements != 1)
    return NULL;
  Item_args tmp(thd, *args);
  return new (thd->mem_root) Item_url_typecast(
    thd, tmp.arguments()[0], thd->variables.collation_connection);
}

bool Type_handler_url::Item_hybrid_func_fix_attributes(
  THD *, const LEX_CSTRING &name, Type_handler_hybrid_field_type *handler,
  Type_all_attributes *func, Item **items, uint count) const
{
  if (func->aggregate_attributes_string(name, items, count))
    return true;
  handler->set_handler(&type_handler_url);
  return false;
}

const Type_handler *
Type_handler_url::type_handler_for_tmp_table(const Item *) const
{ return &type_handler_url; }

void Field_url::sql_type(String &out) const
{ out.set_ascii(STRING_WITH_LEN("url")); }

int Field_url::store(const char *from, size_t length, CHARSET_INFO *cs)
{
  if (url_parse(from, length, NULL))
    return Field_blob::store(from, length, cs);

  my_error(ER_WRONG_VALUE, MYF(0), "URL",
           ErrConvString(from, length, cs).ptr());
  if (maybe_null())
    set_null();
  else
    Field_blob::store(STRING_WITH_LEN("about:invalid"), cs);
  return -1;
}

enum_conv_type Field_url::rpl_conv_type_from(
  const Conv_source &source, const Relay_log_info *, const Conv_param &) const
{
  const Type_handler *type= source.type_handler();
  if (type == &type_handler_tiny_blob || type == &type_handler_medium_blob ||
      type == &type_handler_long_blob || type == &type_handler_blob ||
      type == &type_handler_blob_compressed || type == &type_handler_string ||
      type == &type_handler_var_string || type == &type_handler_varchar ||
      type == &type_handler_varchar_compressed)
    return CONV_TYPE_PRECISE;
  return CONV_TYPE_IMPOSSIBLE;
}

class Item_url_typecast_handler: public Item_handled_func::Handler_str
{
public:
  const Type_handler *return_type_handler(
    const Item_handled_func *) const override { return &type_handler_url; }
  const Type_handler *type_handler_for_create_select(
    const Item_handled_func *) const override { return &type_handler_url; }
  bool fix_length_and_dec(Item_handled_func *) const override { return false; }
  String *val_str(Item_handled_func *item, String *to) const override
  {
    return static_cast<Item_url_typecast *>(item)->val_str_generic(to);
  }
};

static Item_url_typecast_handler item_url_typecast_handler;

LEX_CSTRING Item_url_typecast::func_name_cstring() const
{
  static LEX_CSTRING name= {STRING_WITH_LEN("cast_as_url")};
  return name;
}

bool Item_url_typecast::fix_length_and_dec(THD *)
{
  Item_char_typecast::fix_length_and_dec_str();
  set_func_handler(&item_url_typecast_handler);
  if (cast_charset()->mbminlen > 1)
  {
    my_error(ER_NOT_SUPPORTED_YET, MYF(0),
             "CAST(AS URL CHARACTER SET ucs2/utf16/utf32)");
    return true;
  }
  return false;
}

String *Item_url_typecast::val_str(String *to)
{
  String *value= Item_char_typecast::val_str(to);
  if (!value || !url_parse(value->ptr(), value->length(), NULL))
  {
    null_value= true;
    return NULL;
  }
  return value;
}

void Item_url_typecast::print(String *out, enum_query_type query_type)
{
  out->append(STRING_WITH_LEN("cast("));
  args[0]->print(out, query_type);
  out->append(STRING_WITH_LEN(" as url"));
  print_charset(out);
  out->append(')');
}
