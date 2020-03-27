## Managing databases
- [ ] `migrate <size>...` -- change column sizes to `<size>...`
- [ ] `create name <column size>...` -- create new database in file `name.txt`, with columns/sizes as given
- [ ] `copy name` -- copy entries from database `name` (ex: `copy sample-database.txt`)

## Editing/quering database
- [ ] `add <value...>` -- add a row with given values, prints it's `idx`
- [ ] `print` -- print whole database in console
- [ ] `find <col_idx> <value>` -- find all entries where column `#col_idx` is equal to `value`
- [ ] `update <row_idx> <col_idx> <value>` -- set value in column `#col_idx` to `value` in row `#row_idx`
- [ ] `remove <idx>` -- mark row as deleted (won't affect other row's indices)
