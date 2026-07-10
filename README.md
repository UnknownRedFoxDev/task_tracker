# Task Tracker - tatr

## Disclaimer

Original idea by [@Tsoding](https://github.com/tsoding) (aka [@Rexim](https://github.com/rexim))  
This is only a recreation for myself, it is open-source but not open-contributions, fork it and do whatever you want.  
And once I'll reach a position where I'm comfortable with this tool, I'll start to add my own idea. This is not meant to be a 1:1 copy to Tsoding's (even more since I don't even know how he implemented his own).  

Moreover, up until [this commit](https://github.com/UnknownRedFoxDev/task_tracker/tree/73bd3c513562d36f5b7fd263c2eb14428c467913), the history is only a sort of replay of what I had previously done.  
Since I forgot to initialize a repo before starting the project, only everything after said commit is progress done in real time.  

This project did not, and will not use any form agentic coding/vibe coding. It was realized by hand and will continue to be done as such.  

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

List of feature available:

`./tatr help`: See help message for more information  
`./tatr init`: initializes the current working directory with a tasks/ directory if it doesn't yet exists  
`./tatr edit <task-id>`: Opens in your $EDITOR (or default to vim) the task  
`./tatr find <task-id>`: Finds and prints the task for quick access  
`./tatr cat <task-id>`: Print a task's details. Avoids having to edit it just to see what's written.  
`./tatr ls [<filters>] ["<name>"]`: Prints every task that answers by the query given. Filtering by tag and by name are **mutually exclusive**.  
`./tatr new [OPTIONS] "<title>"`: Creates a new task and opens in your `$EDITOR`, defaults to vim.  
`./tatr (rm|del) <task-id> [...]`: Deletes the task(s) specified  
`./tatr close <task-id> [...]`: Closes the task(s)  
`./tatr (summary|sum)`: See the different stats related to the task available


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

- `.<tag1>`             : Filter by tasks having at least `tag1`  
- `.<tag1> and .<tag2>` : Filter tasks having at least `tag1` AND `tag2`  
- `.<tag1> or .<tag2>`  : Filter tasks having at least `tag1` OR `tag2`  
- `not .<tag1>`         : Filter by tasks which do not have `tag1`  


Some predefined tags:  

`.all`      : both .OPEN and .CLOSED tasks  
`.OPEN`     : every opened tasks (default)  
`.CLOSED`   : every closed tasks  
`.TAGGED`   : every tagged tasks  
`.UNTAGGED` : every untagged tasks  

#### Example

```bash
./tatr ls .CLOSED and .UNTAGGED "test untagg" # every closed and untagged tasks containing in their name "test untagg"
```

```bash
./tatr ls not .CLOSED # every opened tasks
```


### By name (tatr ls "[name]")

The name specified doesn't have to be an exact match.  
It will "fuzzy-find" all the tasks having the <name> given.  

#### Example

```bash
./tatr ls "test" # every tasks containing in their name "test"
```
