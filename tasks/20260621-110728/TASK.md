# Replace filter system to use .<str> for tag filtering and tokenized those to use 'and' and 'or' and 'not'

- STATUS: OPEN
- PRIORITY: 80
- TAGS: feature,filters,tags-system,internal-rework

User should be able to call this program as such:

```bash
./tatr ls [.<tag1> [and .<tag2> [or .<tag3> [and not .<tag4> [or not .<tag5>] ] ] ] ]
```

In terms, the filtering should represent a sort of tokenization that looks for tasks with such tags.
Keywords here are `and`, `or` and `not`.

Description of each keywork:

- `.<tag1>` : Says, filter by tasks having at least `tag1`
- `.<tag1> and .<tag2>` : Says, filter tasks with **only** `tag1` AND `tag2`
- `.<tag1> or .<tag2>` : Says, filter tasks with at least `tag1` OR `tag2`
- `not .<tag1>` : Says, filter by tasks which do not have `tag1`

Related tasks:

- TASK(20260621-112041): Rework tag system
- TASK(20260621-115953): Add additional filtering keyworks
