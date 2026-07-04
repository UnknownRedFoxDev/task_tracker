# Allow enclosing tags with parenthesis

- STATUS: CLOSED
- PRIORITY: 80
- TAGS: internal-rework,feature,filters,bug

# Current issue
Queries like:

- `.all and .feature or .debug`

would logically translate like:

.OPEN and .CLOSED and .feature or .debug

In order:

tasks with .OPEN tag
and
tasks with .CLOSED tag
and
tasks with .feature

combining with

tasks with .debug

## General concensus
so something akin to `(.OPEN and .CLOSED and .feature) or .debug`
but we would have wanted `(.OPEN and .CLOSED) and (.feature or .debug)`

- `.OPEN and .feature or .debug`
Would result in `(.OPEN and .feature) or .debug`
when we would have wanted `.OPEN and (.feature or .debug)`


## What needs to be done
So we need to put implicit parenthesis around the expression we want to evaluate "first". Not really first, but that we group togather as `and` has more precedence over `or`.

# What could be done
So far we two functions: eval_tokens and eval_token, so what we need is a third function "eval_expression"

We define an expression as combinaison of two tokens. A query is a series of expressions and expressions are made out of tokens.

By default, the query is as simple as a single token `.OPEN`.
When we add something: `.feature`, the query because an expression of `.OPEN and .feature`

To be noted, when the query has more than one token, unless `.all` or `.CLOSED` keywords are detected, it should automatically, behind the scene, add parenthesis around the query, as such:

Query: `.feature or .debug`
Intermidiate query: `.OPEN and .feature or .debug`
Final query: `.OPEN and (.feature or .debug)`

The process could go as:

parse the expression:

tasks of .OPEN
and
tasks of (
      tasks of .feature
      or
      tasks of .debug
    )
# The bigger picture

This change is bigger than expected and actually needs me to totally rework the parsing system:

First step is to re-organize the query in a tree like representation:

.CLOSED and (.feature or .bug) ->                and
                                              /       \
                                       .CLOSED         or
                                                     /    \
                                             .feature      .bug

                                    = and .CLOSED or .feature .bug

Basically that or like the Reverse Polish Notation: .feature .bug or .CLOSED and

A token:

struct Token {
    val: const char *;
    type: Token_Type;
};

enum Token_Type {
    TOKEN_TAG,
    TOKEN_NOT,
    TOKEN_AND,
    TOKEN_OR,
};

struct Tokens {
    items: Token *;
    count: size_t;
    capacity: size_t;
};

When parsing the query:
- `.CLOSED and .feature`

encounter: .CLOSED  -> starts with a '.' == TOKEN_TAG
encounter: and      -> TOKEN_AND
encounter: .feature -> starts with a '.' == TOKEN_TAG

when 'and' is encountered:
prev: .CLOSED
next: .feature

tokens: ["and", ".CLOSED", ".feature"]
=              and
      .CLOSED       .feature

- `.CLOSED and (.feature or .bug)`

encounter: .CLOSED  -> starts with a '.' == TOKEN_TAG
encounter: and      -> TOKEN_AND
tokens: ["and", ".CLOSED"]

encounter: (.feature -> starts with a '(':
    remove '(' and start back from this tag until matching ')' is found
    encounter: .feature -> starts with a '.' == TOKEN_TAG
    encounter: or       -> TOKEN_OR
    encounter: .bug)    -> ends with ')', starts with '.' == TOKEN_TAG
    new_tokens: ["or", ".feature", ".bug"]

tokens: ["and", ".CLOSED", "or", ".feature", ".bug"]
=                   and
        .CLOSED              or
                     .feature  .bug

- `.CLOSED and .feature or .bug`

encounter: .CLOSED  -> starts with a '.' == TOKEN_TAG
encounter: and      -> TOKEN_AND
encounter: .feature -> starts with a '.' == TOKEN_TAG

when 'and' is encountered:
prev: .CLOSED
next: .feature
tokens: ["and", ".CLOSED", ".feature"]

encounter: or  -> TOKEN_OR
encounter: .bug -> starts with a '.' == TOKEN_TAG

when 'and' is encountered:
prev: .feature
next: .bug

tokens: ["and", ".CLOSED", ".feature", "or", ".bug"]
expected: ["or", ".bug", "and", ".feature", ".CLOSED"]
=            or
  .bug                and
             .feature     .CLOSED

but it should give: or .bug and .feature .CLOSED

It searches the tree in prefix
To note that `and` has more precendence over `or`

## Consideration

a & b | c ~= (a & b) | c
a & (b | c)

prev: a
curr: &
next: (b | c) from sb-to-sv
|
| Recursive call?
   i.e. eval_tokens (tokens, sv)
v
prev: b
curr: |
next: c

the expression inside parenthesis need to equate 0 in parenthesis sum ['(': +1, ')': -1] where the expression needs to have as much opening parenthesis as closing ones. Once it has achieved it, it does the recursive call with the expression built into a string builder turned into a string view.

# Issue with all of that
Right now, with the current system, it works but only sorta. It evaluate as it goes and doesn't build a tree (AST tree). Meaning that putting `.CLOSED and .feature or .bug` gives an array of closed tasks with feature as a tag, but also both opened and closed tasks with bug tag. But we want a result of closed task with either feature or bug in their tags.

The above consideration is a work around to be lazy and not build an ast tree and go with eval as you go approach.
