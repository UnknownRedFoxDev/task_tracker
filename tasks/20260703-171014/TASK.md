# Change query evaluation method from eval as you to building an ast tree and go from there

- STATUS: OPEN
- PRIORITY: 70
- TAGS: internal-rework,filters

Rather than continue with eval the query as you go approach, this task's goal is to rework the system in order to build an AST tree and build the result array while traversing the tree in, seemingly, the prefix order.

See task(20260703-022205) for more information.

.CLOSED and (.feature or .bug)
Tree:
             and
          /       \                                                .CLOSED         or                                                            /    \                                                  .feature      .bug

= and .CLOSED or .feature .bug
