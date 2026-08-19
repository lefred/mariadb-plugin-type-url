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
#include "sql_type_url.h"
#include "item_urlfunc.h"
#include <mysql/plugin_data_type.h>
#include <mysql/plugin_function.h>

static st_mariadb_data_type type_descriptor=
{
  MariaDB_DATA_TYPE_INTERFACE_VERSION,
  &type_handler_url
};

template <Url_component component>
class Create_url_component: public Create_func_arg1
{
public:
  Item *create_1_arg(THD *thd, Item *arg) override
  {
    return new (thd->mem_root) Item_func_url_component(thd, arg, component);
  }
};

static Create_url_component<URL_COMPONENT_SCHEME> create_url_scheme;
static Create_url_component<URL_COMPONENT_HOST> create_url_host;
static Create_url_component<URL_COMPONENT_PATH> create_url_path;
static Plugin_function function_url_scheme(&create_url_scheme);
static Plugin_function function_url_host(&create_url_host);
static Plugin_function function_url_path(&create_url_path);

#define URL_PLUGIN(kind, descriptor, plugin_name, description) \
  { kind, descriptor, plugin_name, "lefred", description, \
    PLUGIN_LICENSE_GPL, 0, 0, 0x0100, NULL, NULL, "0.2.0", \
    MariaDB_PLUGIN_MATURITY_BETA }

maria_declare_plugin(type_url)
  URL_PLUGIN(MariaDB_DATA_TYPE_PLUGIN, &type_descriptor, "url",
             "Validated URL data type"),
  URL_PLUGIN(MariaDB_FUNCTION_PLUGIN, &function_url_scheme, "url_scheme",
             "Extract the scheme from a URL"),
  URL_PLUGIN(MariaDB_FUNCTION_PLUGIN, &function_url_host, "url_host",
             "Extract the host from a URL"),
  URL_PLUGIN(MariaDB_FUNCTION_PLUGIN, &function_url_path, "url_path",
             "Extract the path from a URL")
maria_declare_plugin_end;
