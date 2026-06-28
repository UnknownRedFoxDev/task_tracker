# Eval tag filter in a non-destructive way

- STATUS: CLOSED
- PRIORITY: 100
- TAGS: filters,internal-rework

Say:

```bash
tatr ls .feature
```

internally it gives:

`.OPEN and .feature`

as it steps through, it first puts the result of `.OPEN` in the result array.
when `and` is encountered, it reaches out for the next token, here `.feature`.
When parsing `.feature`, it puts into the HTable every task that has both `.OPEN` and `.feature` as tags.

However, if we'd have:

```bash
tatr ls not .feature
```

internally we have:

`.OPEN and not .feature`

But, we've yet to clear out the case where it encounters `not` after `and`.

# Proposition

We could create a helper function that evals a tag usings the tasks inside the tasks_t *, with a flag `not` to be set either to `true` or `false`.
This returns an array of pointers to the tasks that validated the tag. Once the array is no longer used, we free it. Freeing just deletes the array. The pointers inside that array are still valid afterwards, so we do not delete them.

## Edit

it was observed that cases handling `and` and `or` which feature a `not` somewhere, either for the previous token before the keyword or the next token after the keyword, needed to retain the current mode `and`/`or`, while retaining the previous and after modes.

it was also found that the next token has to be parsed before anything in order to found its mode, if any (aka `NOT` mode), as to account for it when putting the results of the tasks found in the HTable depending on the next mode established.
