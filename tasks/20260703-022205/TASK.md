# Allow enclosing tags with parenthesis

- STATUS: OPEN
- PRIORITY: 80
- TAGS: internal-rework,feature,filtering

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
So we need to put implicit parenthesis around the expression we want to evaluate "first". Not really first, but that we group together as `and` has more precedence over `or`.

# What could be done
So far we two functions: eval_tokens and eval_token, so what we need is a thrid function "eval_expression"

We define an expression asa combinaison of two tokens. A query is a series of expressions and expressions are made out of tokens.

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

It's 2 am, nearly 3 am, when you wake up, go search how they parse if expression while taking into account parenthesis, and remember, **no AI**, not even the preview. Always in `-ai` in your queries.
