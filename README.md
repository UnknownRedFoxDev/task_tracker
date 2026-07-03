# Task Tracker - tatr

## Disclaimer

Original idea by [@Tsoding](https://github.com/tsoding) (aka [@Rexim](https://github.com/rexim))  
This is only a recreation for myself, it is open-source but not open-contributions, fork it and do whatever you want.  
And once I'll reach a position where I'm comfortable with this tool, I'll start to add my own idea. This is not meant to be a 1:1 copy to Tsoding's (even more since I don't even know how he implemented his own).  

Moreover, up until [this commit](d0eba0227b83d3d369d1194adbaf387e0d5ba477), the history is only a sort of replay of what I had previously done.  
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

```bash
./tatr ls # Does the same but it allows filtering
```

`./tatr help`: See help message for more information  
`./tatr init`: initializes the current working directory with a tasks/ directory if it doesn't yet exists  
`./tatr edit <task-id>`: Opens in your $EDITOR (or default to vim) the task  
`./tatr find <task-id>`: Finds and prints the task for quick access  
`./tatr new [OPTIONS] "<title>"`: Creates a new task and opens in your `$EDITOR`, defaults to vim. See help message for more information  
`./tatr (rm|del) <task-id> [<task-id> [...] ]`: Deletes the task(s) specified  
`./tatr close <task-id> [<task-id> [...] ]`: Closes the task(s)  
`./tatr (summary|sum)`: See the different stats related to the task available


## Filtering the tasks

```bash
./tatr ls [not] <tag1> [and|or [not] <tag2>] ["<name>"]
```
Fuzzy-finds every task that answers to the query specified

### By tag

Originally, it was meant to do all the filtering like by name  
Though, Tsoding's way to seperate tags like `.<tag>` seems more intuitive  
This allows for a different approach, a more permissive approach  

```bash
./tatr ls [.<tag1> [and .<tag2> [or .<tag3> [and not .<tag4> [or not .<tag5>] ] ] ] ]
```

This works with keywords. Keywords here are `and`, `or` and `not`  

- `.<tag1>`             : Filter by tasks having at least `tag1`  
- `.<tag1> and .<tag2>` : Filter tasks having `tag1` AND `tag2`  
- `.<tag1> or .<tag2>`  : Filter tasks having at least `tag1` OR `tag2`  
- `not .<tag1>`         : Filter by tasks which do not have `tag1`  


Some predefined tags:  

`all` : both .OPEN and .CLOSED tasks  
`.OPEN` : every opened tasks (default)  
`.CLOSED` : every closed tasks  
`.TAGGED` : every tagged tasks  
`.UNTAGGED` : every untagged tasks  

#### Example

```bash
./tatr ls .CLOSED and .UNTAGGED "test untagg" # every closed and untagged tasks containing in their name "test untagg"
```

```bash
./tatr ls not .CLOSED # every opened tasks
```

```bash
./tatr ls "test" # every opened tasks containing in their name "test"
```
