# Allow ls filtering to use the task's name too

- STATUS: OPEN
- PRIORITY: 60
- TAGS: filters,internal-rework

Previously, the system only revolved around filtering by name using strstr.
Afterwards, it grew into using strstr for the status, tags and name.
Now, it only searches through the pre-defined tags and the tasks' tags too.

However, now I want to be able to do something like:

```bash
tatr ls .cmdline-options "init"
```

Where starting with a double or singular quote, the user wants to filter by the name too.
As such, with the current system in place, grow it to also support name.

# Possible changes

As of right now, the query is decomposed into tokens that create tables of tags answering to the tag.
At the end everything is re-grouped into a singular result array that is then printed.

The name based filtering could take advantage of the current tag token evaluation to for example, add a bool to indicate that it is the name evaluation.
Leading to the function creating an array of tasks having the name string in their task's name.

Ultimately, it will be using `strstr()` in order to be efficient.

# Corner cases

There is now the issue of distinguishing the tag filtering to the name filtering.
Although, as seen above, the string is seperated by double quotes (singular quote would also work), and when that's the case, it should immediately go for name filtering rather than tag filtering.

## How does it work with and/or

The though of trying to filter tags then using `and` and then using the name filtering is a bit dump in sense since it's basically what is done by default.
It should probably just discard the and if the next token `starts_with()` a quote or double quote and immediately start parsing the name while entering the and/or logic. Albeit, a bit altered then.
