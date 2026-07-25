# Copyright (c) 2019, MariaDB corporation. All rights reserved.
# Copyright (c) 2026, lefred (Frédéric Descamps). All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1335  USA

MYSQL_ADD_PLUGIN(type_url
                 plugin.cc item_urlfunc.cc sql_type_url.cc url_parser.cc
                 MODULE_ONLY RECOMPILE_FOR_EMBEDDED)

ADD_EXECUTABLE(type_url_parser_test EXCLUDE_FROM_ALL
               tests/url_parser_test.cc url_parser.cc)
TARGET_INCLUDE_DIRECTORIES(type_url_parser_test PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR})
ADD_TEST(NAME type_url_parser COMMAND type_url_parser_test)

