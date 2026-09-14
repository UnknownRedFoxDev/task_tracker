# Task Tracker - tatr

## Disclaimer

Original idea and implementation by [@Tsoding](https://github.com/tsoding): [here](https://github.com/tsoding/tatr/)  

This repo is an implementation of mine before he released his version to the public.  
It is in no-way a 1:1 to his, that it'd be in implementation, design, or usage of the tool, not saying it won't feel similar because it will. I've made adjustments to my own taste, good or bad, tailored for me.  

You can find the specs of the tool and its query language [here](https://github.com/tsoding/tatr/blob/README.md).  

This project was and is made as a recreational side-project, no agentic tool were used, otherwise there is no reason to do any of that. Just use Tsoding's version if you prefer how he did his.

## How to use

### Compile

```bash
cc -o nob nob.c
./nob
```

### The tool

The default behavior is to list every opened task available in their order of priority. The big the priority, the higher it will be.  

```bash
./tatr
```

List of features available:
| name | description |
| - | - |
| help | print the help message |
| version | Displays relevant version information about the tooling |
| init [y] | Initialise the tasks directory if not already present. `y` option is to force creation |
| ls [OPTIONS] | Lists all tasks. Filters as strings can be passed to filter tasks by name, status and tags. Filtering by tag and by name are mutually exclusive |
| ls-rev [OPTIONS] | Same as `ls`, but reverses the output's order |
| edit \<task-huid\> | Opens in your $EDITOR (or default to vim) the task |
| find \<task-huid\> | Finds and prints the task for quick access |
| cat \<task-huid\> | Print a task's details. Avoids having to edit it just to see what's written |
| new [OPTIONS] "\<title\>" | Creates a new task and opens in your $EDITOR, unless --no-editor is specified, defaults to vim |
| rm \| del [-last \<int\>] \<task-huid\> [...] | Deletes the specified task(s), or the n last opened tasks with `-last` flag |
| reopen \<task-huid\> [...] | Closes the specified task(s) |
| sum[mary] | Prints stats info |
| overwrite [-t [+|-]\<tags\>[,...]] [-p [+|-]\<priority\>] [-s <O[PEN] | C[LOSED]>] [title] \<task-huid [...] | query\> | Modify a or multiple tasks' tags, priority, title or status at once |

## Filtering the tasks (tatr ls)

### Disclaimer
Filtering by tag and by name are mutually exclusive. The implementation to filter for both was clunky and the use-cases for that trouble are non-existent (for me).  
So you can either filter by tag, OR by the name of the task(s). But not both at the same time.  

### By tag (tatr ls [tags])

Originally, it was meant to do all the filtering like by name. Though, Tsoding's way to seperate tags like `.<tag>` seems more intuitive.  
This allows for a different approach, a more permissive one at that.  

```bash
./tatr ls [.<tag1> [and .<tag2> [or .<tag3> [and not .<tag4> [or not .<tag5>] ] ] ] ]
```

This works with keywords. Keywords here are `and`, `or` and `not`  

- `.<tag1>`             := Filter by tasks having at least `tag1`  
- `.<tag1> and .<tag2>` := Filter tasks having at least `tag1` AND `tag2`  
- `.<tag1> or .<tag2>`  := Filter tasks having at least `tag1` OR `tag2`  
- `not .<tag1>`         := Filter by tasks which do not have `tag1`  
- `[expr]`              := Group tag expression inside "[" "]"

Some predefined tags:  

`.all`      := both .OPEN and .CLOSED tasks  
`.OPEN`     := every opened tasks (default)  
`.CLOSED`   := every closed tasks  
`.TAGGED`   := every tagged tasks  
`.UNTAGGED` := every untagged tasks  

#### Example

```bash
./tatr ls .CLOSED and .UNTAGGED "test untagg" # every closed and untagged tasks containing in their name "test untagg"
```

```bash
./tatr ls not .CLOSED # every opened tasks
```

```bash
./tatr ls .all and [.bug and not [.complex or .important]] # for every tasks, if they contain bug, while not having complex or important
# equivalent to
./tatr ls .all and [.bug and [not .complex or not .important]] # for every tasks, if they contain bug, while not having complex or important
```

```bash
./tatr ls not [not [not [not [not [not .urmom]]]]] # every opened task that did urmom
```


### By name (tatr ls "[name]")

The name specified doesn't have to be an exact match.  
It will "fuzzy-find" all the tasks having the <name> given.  

#### Example

```bash
./tatr ls "test" # every tasks containing in their name "test"
```
