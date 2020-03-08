## Managing databases
- [ ] `migrate <size>...` -- change column sizes to `<size>...`
- [ ] `create name <column size>...` -- create new database in file `name.txt`, with columns/sizes as given

## Editing/quering database
- [+] `add <value...>` -- add a row with given values, prints it's `idx`
- [ ] `print` -- print whole database in console
- [ ] `update <row_idx> <column> <value>` -- set value in `column` to `value` in row `#row_idx`
- [ ] `remove <idx>` -- mark row as deleted (won't affect other row's indices)