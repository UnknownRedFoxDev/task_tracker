# Implement overwrite cmdline

- STATUS: CLOSED
- PRIORITY: 110
- TAGS: cmdline-options,feature

`tatr overwrite <task-id> [-t [+|-]<tags> ...] [-p [+|-]<priority>] [-s <O[PEN] | C[LOSED]>] [title]`
Given a task-huid, you can modify its tags, priority, status and title.

`tatr overwrite <huid> -p 90` will overwrite the task's priority to 90.
`tatr overwrite <huid> -p -10` will overwrite the task's priority to decrease it by 10.
`tatr overwrite <huid> -p +20` will overwrite the task's priority to increase it by 20.
`tatr overwrite <huid> -t bug` will overwrite the task's tags to just "bug".
`tatr overwrite <huid> -t +bug` will add "bug" to the task's tags.
`tatr overwrite <huid> -t -feature` will remove "feature" to the task's tags, if it exists.
`tatr overwrite <huid> -s O` will overwrite the task's status to OPEN.
`tatr overwrite <huid> -s C` will overwrite the task's status to OPEN.
`tatr overwrite <huid> "new title"` will overwrite the task's title to "new title".

All of the cmdline options are not exclusive to each other.
Meaning you could do this:

`tatr overwrite <huid> -p -20 -t +feature "exclusive title"` which will overwrite the task's priority by decreasing it by 20, add feature to its tags, and change the title to "exclusive title"
`tatr overwrite <huid> -p 55 -t -bug -s CLOSED` which will overwrite the task's priority setting it to 55 and remove bug to its tags

(sorry to future me to have planned such a huge ass cmdline option at 2am lmao)

