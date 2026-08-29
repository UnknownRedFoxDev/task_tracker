# Allow query to filter the priorities

- STATUS: OPEN
- PRIORITY: 130
- TAGS: internal-rework,query-lang

The basic idea:

```
tatr ls priority lt 50 and .feature
```

```
tatr ls priority lt 50 and priority gte 20
```

This should look for the task's priority, and see if it's in the range.
A range can be create/set with min and max values.

By default those would be 0 and the task with the highest priority.
Flags could be set such that min-eq and max-eq when doing lte (lesser than or equal to) and gte (greater than or equal to) which would make the range inclusive rather than exclusive

e.g.

#### example 1
min = 30  ; min-eq = false
max = 100 ; max-eq = false

range = (30, 100)
Include tasks with priority between: 31; 99

#### example 2
min = 12 ; min-eq = false
max = 95 ; max-eq = true

range = (12, 95]
Include tasks with priority between: 13; 95

#### example 3
min = 65 ; min-eq = true
max = 90 ; max-eq = false

range = [65, 90)
Include tasks with priority between: 65; 89

#### example 4
min = 60 ; min-eq = true
max = 70 ; max-eq = true

range = [60, 70]
Include tasks with priority between: 60; 70
