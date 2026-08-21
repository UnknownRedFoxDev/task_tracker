# Change query evaluation method from eval as you go to building an ast tree and go from there

- STATUS: OPEN
- PRIORITY: 140
- TAGS: internal-rework,filters,feature

Rather than continue with eval the query as you go approach, this task's goal is to rework the system in order to build an AST tree and build the result array while traversing the tree in, seemingly, the prefix order.

See task(20260703-022205) for more information.

.CLOSED and (.feature or .bug)
Tree:
             and
          /       \
    .CLOSED         or
                  /    \
          .feature      .bug

= and .CLOSED or .feature .bug

A way to represent the AST:

AND:
    .CLOSED
    OR:
        .feature
        .bug

.feature and not .bug:
AND:
    .feature
    NOT .bug

.tests or (.bug and not .feature):
OR:
    .tests
    AND:
        .bug
        not .feature

.tests or .bug and not .feature: (`and` has a higher precedence)
AND:
    not .feature
    OR:
        .tests
        .bug


But more importantly it will look like:

OP: AND
    LHS:
        TAG: .feature
            NEGATION: TRUE
    RHS:
        OP: OR
            LHS:
                TAG: .test
                    NEGATION: FALSE
            RHS:
                TAG: .bug
                    NEGATION: FALSE
This shows two mergent behaviors:

1. OPERATOR (OP) have two sides, the left-hand-side (LHS) and the right-hand-side (RHS)
2. A TAG has a negation field either true if not is before it, false otherwise
