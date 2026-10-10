# INSTALL

```sql
MariaDB > INSTALL SONAME 'type_url';
```

Confirmation:

```sql
MariaDB > SELECT plugin_name, plugin_type, plugin_library, plugin_description,
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

# USAGE

Basic usage:

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
