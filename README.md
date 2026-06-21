# Task Tracker - tatr

## Disclaimer

Original idea by [@Tsoding](https://github.com/tsoding) (aka [@Remix](https://github.com/rexim))
This is only a recreation for myself, it is open-source but not open-contributions, fork it and do whatever you want.
And once I'll reach a position where I'm comfortable with this tool, I'll start to add my own idea. This is not meant to be a 1:1 copy to Tsoding's (even more since I don't even know how he implemented his own).

Moreover, up until [this commit](d0eba0227b83d3d369d1194adbaf387e0d5ba477), the history is only a sort of replay of what I had previously done.
Since I forgot to initialize a repo before starting the project, only everything after said commit is progress done in real time.

This project did not, and will not use any form of AI to conjure idea, write code, create test cases, or any other form of help one could get from them.
It was realized by hand and will continue to be done as such.


## How to use

### Compile

```bash
cc -o nob nob.c
./nob
```


### Use the tool

The default behavior is to list every opened task available in their order of priority. The big the priority, the higher it will be.

```bash
./tatr
```

```bash
./tatr ls
```

`./tatr help`: See help message for more informations.
`./tatr open <task-id>`: Opens in your $EDITOR (or default to vim) the task.
`./tatr close <task-id> [<task-id> [...] ]`: Closes the task(s).
`./tatr find <task-id>`: Finds and prints the task for quick access.
`./tatr create "<title>"`: Creates a new task and opens in your `$EDITOR`.
`./tatr rm <task-id> [<task-id> [...] ]`: Deletes the task(s) specified.
`./tatr summary`: See the different stats related to the task available.
                  Total amount of task, those opened, closed or untagged, and then every tag with their count associated.


#### Filtering by name

```bash
./tatr ls <task-name>
```

This will show every task having the name inputted, using a sort of fuzzy-finding.


#### Filtering by tag

Originally, it was meant to do all the filtering like by name.
Though, Tsoding's way to seperate tags like `.<tag>` seems more intuitive.
This allows for a different approach, a more permissive approach.

```bash
./tatr ls [.<tag1> [and .<tag2> [or .<tag3> [and not .<tag4> [or not .<tag5>] ] ] ] ]
```

This works with keywords. Keywords here are `and`, `or` and `not`.

- `.<tag1>`             : Filter by tasks having at least `tag1`.
- `.<tag1> and .<tag2>` : Filter tasks with **only** `tag1` AND `tag2`.
- `.<tag1> or .<tag2>`  : Filter tasks with at least `tag1` OR `tag2`.
- `not .<tag1>`         : Filter by tasks which do not have `tag1`.


##### Filtering by pre-defined tags

Filter by opened tasks. Aka, the default.
```bash
./tatr ls .OPEN
```

Filter by closed tasks.
```bash
./tatr ls .CLOSED # basically `not .OPEN`
```

Filter by untagged tasks.
```bash
./tatr ls .UNTAGGED
```

###
