# allow `close`, `reopen`, `overwrite` and `rm` to "use" the query language

- STATUS: OPEN
- PRIORITY: 60
- TAGS:

Rather than giving the tasks huid directly, the query language could be used.

For their last argument, search for at least one task, if not found, change to directly use the query parsing
i.e. first group every argument into a string then use that string to parse the query

Alternative is to create three new command line options:

`tatr tag -t <tag>[,...] <huid [...] | query>`
`tatr untag -t <tag>[,...] <huid [...] | query>`
`tatr priority -p <int> <huid [...] | query>`


# Related tasks
TASK(20260910-201041): the query should be assembled argument per argument into a single string then parsed
