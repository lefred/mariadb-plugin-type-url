# mariadb-plugin-type-url

![mariabd-plugin-uuid-time](logo/url_type.png)

This plugin adds a validated `URL` data type and three deterministic SQL
functions:

- `URL_SCHEME(url)` returns the URI scheme.
- `URL_HOST(url)` returns the host without userinfo, brackets, or port; it
  returns `NULL` for schemes without an authority component.
- `URL_PATH(url)` returns the path without query or fragment.

Values must be absolute RFC 3986-style URIs. Validation rejects whitespace and
control characters, malformed percent escapes, empty/malformed authorities,
unbracketed IPv6 literals, and ports outside `0..65535`. Validation is
syntactic: it does not perform DNS resolution or scheme-specific normalization.

## Build and load

Place this directory under `plugin/type_url` in a MariaDB server source tree,
then configure and build the loadable module:

```sh
cmake -S . -B build -DPLUGIN_TYPE_URL=DYNAMIC
cmake --build build --target type_url
```

The resulting shared object is
`build/plugin/type_url/type_url.so`. Install it in the server's plugin
directory (the directory reported by `SELECT @@plugin_dir`) using the normal
installation target or by copying the shared object there.

Load all four plugin entries—the datatype and its three functions—with one
statement:

```sql
INSTALL SONAME 'type_url';
```

Confirm that they loaded:

```sql
SELECT plugin_name, plugin_type, plugin_library, plugin_description,
plugin_author FROM information_schema.PLUGINS WHERE plugin_library LIKE 'type_url.so';
+-------------+-------------+----------------+-------------------------------+---------------+
| plugin_name | plugin_type | plugin_library | plugin_description            | plugin_author |
+-------------+-------------+----------------+-------------------------------+---------------+
| url         | DATA TYPE   | type_url.so    | Validated URL data type       | lefred        |
| url_scheme  | FUNCTION    | type_url.so    | Extract the scheme from a URL | lefred        |
| url_host    | FUNCTION    | type_url.so    | Extract the host from a URL   | lefred        |
| url_path    | FUNCTION    | type_url.so    | Extract the path from a URL   | lefred        |
+-------------+-------------+----------------+-------------------------------+---------------+
4 rows in set (0.002 sec)
```

`INSTALL SONAME` persists the plugins in `mysql.plugin`, so MariaDB loads them
again after a restart. To remove them, first drop or convert columns that use
the `URL` datatype, then run:

```sql
UNINSTALL SONAME 'type_url';
```

## Testing

The URL-parsing logic in `url_parser.cc` has a standalone unit test that
builds independently of the MariaDB server (`tests/url_parser_test.cc`). It
links only `url_parser.cc` against a small `assert()`-based `main()`, so it
compiles and runs in seconds and is useful for iterating on the parser
itself:

```sh
cmake --build build --target type_url_parser_test
ctest --test-dir build -R type_url_parser
```

For end-to-end coverage of the SQL-visible behavior (the `URL` column type,
`CAST(... AS URL)`, and the `URL_SCHEME`/`URL_HOST`/`URL_PATH` functions
against a running server), use the `mysql-test/type_url` suite from a MariaDB
server source tree:

```sh
cd mysql-test
./mtr type_url
```

## Basic use

```sql
CREATE TABLE links (
  id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
  target URL NOT NULL
);

INSERT INTO links (target) VALUES
  ('https://example.com/docs/start?q=sql#install'),
  ('ftp://downloads.example.org/pub/archive.tar.gz');

SELECT
  target,
  URL_SCHEME(target) AS scheme,
  URL_HOST(target) AS host,
  URL_PATH(target) AS path
FROM links\G

*************************** 1. row ***************************
target: https://example.com/docs/start?q=sql#install
scheme: https
  host: example.com
  path: /docs/start
*************************** 2. row ***************************
target: ftp://downloads.example.org/pub/archive.tar.gz
scheme: ftp
  host: downloads.example.org
  path: /pub/archive.tar.gz
2 rows in set (0.001 sec)
```

The query string and fragment remain part of the stored value but are not
included in `URL_PATH()`.

## Cast and constructor

Strings can be explicitly converted with either `CAST(... AS URL)` or the
datatype constructor:

```sql
SELECT CAST('https://mariadb.org/download/' AS URL);
SELECT URL('https://mariadb.org/kb/en/');

INSERT INTO links (target)
VALUES (CAST('ssh://git@example.com:2222/project/repository' AS URL));
```

An invalid explicit conversion returns `NULL`:

```sql
SELECT CAST('not a URL' AS URL);
+--------------------------+
| CAST('not a URL' AS URL) |
+--------------------------+
| NULL                     |
+--------------------------+
1 row in set (0.001 sec)
```

## Authority, ports, and IPv6

`URL_HOST()` removes credentials, brackets, and the port from the authority:

```sql
SELECT
  URL_SCHEME('https://alice:secret@example.com:8443/private') AS scheme,
  URL_HOST('https://alice:secret@example.com:8443/private') AS host,
  URL_PATH('https://alice:secret@example.com:8443/private') AS path;
+--------+-------------+----------+
| scheme | host        | path     |
+--------+-------------+----------+
| https  | example.com | /private |
+--------+-------------+----------+
1 row in set (0.000 sec)

SELECT
  URL_HOST('http://[2001:db8::1]:8080/index.html') AS host,
  URL_PATH('http://[2001:db8::1]:8080/index.html') AS path;
+-------------+-------------+
| host        | path        |
+-------------+-------------+
| 2001:db8::1 | /index.html |
+-------------+-------------+
1 row in set (0.000 sec)
```

Schemes without an authority have no host. Their scheme-specific value is
returned as the path:

```sql
SELECT
  URL_SCHEME('mailto:user@example.com') AS scheme,
  URL_HOST('mailto:user@example.com') AS host,
  URL_PATH('mailto:user@example.com') AS path;
+--------+------+------------------+
| scheme | host | path             |
+--------+------+------------------+
| mailto | NULL | user@example.com |
+--------+------+------------------+
1 row in set (0.000 sec)
```

## Validation

Validation happens when a value is stored in a `URL` column:

```sql
INSERT INTO links (target) VALUES ('relative/path');
-- ERROR: Incorrect URL value: 'relative/path'

INSERT INTO links (target) VALUES ('https://example.com/%ZZ');
-- ERROR: Incorrect URL value: 'https://example.com/%ZZ'

INSERT INTO links (target) VALUES ('https://example.com:70000/');
-- ERROR: Incorrect URL value: 'https://example.com:70000/'
```

Some representative accepted forms are:

```sql
INSERT INTO links (target) VALUES
  ('http://localhost/'),
  ('https://example.com'),
  ('http://[::1]/health'),
  ('file:/var/lib/data.txt'),
  ('urn:isbn:9780131103627');
```

The plugin validates URL syntax only. It does not verify that a hostname
exists, connect to a server, normalize case, or decode percent-encoded data.
