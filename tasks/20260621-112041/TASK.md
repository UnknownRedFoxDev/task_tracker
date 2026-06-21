# Rework tag system

- STATUS: OPEN
- PRIORITY: 100
- TAGS: feature,tags-system,internal-rework

As of right now, a task is composed as such:

```c
typedef struct {
    const char *path;
    const char *uuid;
    char *name;
    uint8_t priority;
    task_status status;
    tags_t tags;
} task_t;
```

Which is fine, but not so much for the `tags_t` structure, describe as:

```c
typedef char * tag_t;

typedef struct {
    tag_t *items;
    size_t count;
    size_t capacity;
} tags_t;
```

## The current issue
This makes the tags a dynamic array leading to having to iterate through it every time one wants to look at the tag of a task.

While it is not really an issue in-on-itself, the pressing matter comes to filtering tasks by tags. The current filtering system pretty relies on the last filter to get the best distance, using levenshtein algorithm.

## How it works

The current algorithm searches as:

- Tags: it looks for substring in the tags arrays of a task, and sets the best dist to 0 if found.
- Status: it looks for substring in the status of a task, and sets the best dist to 5 if found.
- Name: it looks for a substring in the name of a task, and sets the best dist to 0 if found. Otherwise, fallback to using levenshtein distance to find similar tasks.


To say the least, this algorithm is not very sophiticated. If any of those are found and the dist is less than the hardcoded `9999` distance, it stops the search for the whole task and sets its distance into a temporary array of a structure `foo`(yet to be named):

```c
struct foo{
    size_t dist;
    task_t *task;
};
```

Finally, it sorts the array either by distance if any filtering, otherwise, it sorts by priority of the task.


## Conclusion
This approach, while it works, doesn't really work all that much, because if a filter was found, it stops the search for the others. Which is not really what we want.

If we refer to TASK(20260621-110728), having a more sophisticated system of filtering won't cut it with this current approach.

## New approach 1

A new approach for the tag system is to use hash tables (from [ht.h](github.com/tsoding/ht.h) where each task has a hash table of tags describe as:

```c
// typedef const char * tag;
// or
// typedef char * tag;

Ht(tag, bool) tags = { .hasheq = ht_cstr_hasheq };
```

This allows for every fast seach of tags with `ht_find(tags, <tag>)` which returns a pointer to the value if it exists.
Meaning, to search for a tag, is to look if it exists in the task's ht.

## New approach 2

Pass from c to cpp and use `std::string` with `std::vector` in other to quickly look if a task has a tag in its vector.

### second aproch

Or, use an unordered map to basically do something similar to [New approach 1](#New-approach-1).


## Final conclusion

While the different approach are as valuable as others, it would be good to consider that ht.h and by extension, hash table are already being used to store the different stats about tasks.

### Additionally

Thus, it would also make it easier to get such stats using HTables using soemthing like `ht_next(tags)`, which iirc, returns a pointer to the next tag in the tags, and a `NULL` if nothing is found. (maybe a while loop until it returns `NULL`?)

Scrap that ^.
I've just remembered that we gatehr stats while reading the task's file.
Meaning that it wouldn't really make it easier to gather such stats anyway.
Although, rather than a `da_append(tags, <tag>`, it would probably be `*ht_puts(tags, <tag>) = true` to do something similar.


Related tasks:

- TASK(20260621-110728): Replace filter system to use .<str> for tag filtering and tokenized those to use 'and' and 'or' and 'not'
- TASK(20260621-115953): Add additional filtering keyworks
