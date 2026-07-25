# AI-assisted changes

Model used: **Claude Sonnet 5** (`claude-sonnet-5`), via Claude Code.

## Review

A full review of the plugin (`url_parser.cc/h`, `sql_type_url.cc/h`, `item_urlfunc.cc/h`,
`plugin.cc`, `CMakeLists.txt`, and the test suite) was performed.

One real bug was found: `Field_url::store()` (`sql_type_url.cc`) raised
`ER_WRONG_VALUE` on invalid input, but `mysql-test/type_url/type_url.test`
originally asserted `ER_TRUNCATED_WRONG_VALUE_FOR_FIELD` — the two other
validated/blob-like types in the MariaDB tree (`Field_fbt` used by INET6,
`Field_vector`) push a truncation *warning* via
`push_warning_truncated_value_for_field()` and only turn it into a hard error
under strict SQL mode, whereas this plugin calls `my_error()` unconditionally.

**Resolution:** the repo owner fixed this by updating the test/result files to
assert `ER_WRONG_VALUE`, i.e. keeping the code's actual (and intentional)
"always hard error on invalid URL, no silent substitution" behavior, and
making the test match it. This is a legitimate, self-consistent design choice
(it also matches the error text documented in `README.md`), so no source
change was needed there.

No further functional/correctness bugs were found on a second, careful pass
over `url_parser.cc` (parsing logic, boundary conditions), `sql_type_url.cc`
(type handler, field, typecast), `item_urlfunc.cc/h` (URL_SCHEME/HOST/PATH),
and `plugin.cc` (plugin registration).

## Changes made in this session

`README.md` documents two behaviors that had no test coverage at all:

- `CAST('not a URL' AS URL)` returns `NULL` on invalid input.
- The `URL('...')` datatype constructor function.

Added coverage for both to `mysql-test/type_url/type_url.test`, with matching
expected output added to `mysql-test/type_url/type_url.result`:

```sql
SELECT CAST('not a url' AS URL);
SELECT URL('https://mariadb.org/kb/en/');
```

**Note:** these two `.result` additions were derived from standard MariaDB
conventions (unaliased expression columns are headed with the verbatim query
text, matching the existing `CAST('ftp://example.org/pub/file' AS URL)` case
already in the file) rather than executed against a live built server —
building `mariadbd` from source was not done in this session due to cost.
Please run `mysql-test-run.pl type_url` to confirm before merging.
