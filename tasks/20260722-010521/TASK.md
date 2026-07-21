# Create a tui to manage tasks

- STATUS: OPEN
- PRIORITY: 80
- TAGS: feature,cmdline-option,ui

Create a user-interface for the terminal using something like ncurses.

the layout would be like:

```txt
+-------------------------------+--------------+----------------------------------------------------------+
|          Task Name x          |  priority v  |                         Details                          |
+-------------------------------+--------------+----------------------------------------------------------+
| [X] Create a tui to manage... |      80      | - Created: 2026-07-03 01:22:03                           |
| [ ] <task-033>                |      79      | - STATUS: OPEN                                           |
| [ ] <task-032>                |      78      | - TAGS: feature, cmdline-option, ui                      |
| [ ] <task-031>                |      77      |                                                          |
| [ ] <task-030>                |      76      | Description:                                             |
| [ ] <task-029>                |      75      |                                                          |
| [ ] <task-028>                |      74      | Create a user-interface for the terminal using something |
| [ ] <task-027>                |      74      | like ncurses.                                            |
| [ ] <task-026>                |      73      |                                                          |
| [ ] <task-025>                |      72      |                                                          |
| [ ] <task-024>                |      71      |                                                          |
| [ ] <task-023>                |      70      |                                                          |
| [ ] <task-022>                |      69      |                                                          |
| [ ] <task-021>                |      68      |                                                          |
| [ ] <task-020>                |      67      |                                                          |
| [ ] <task-019>                |      66      |                                                          |
| [ ] <task-018>                |      65      |                                                          |
| [ ] <task-017>                |      64      |                                                          |
| [ ] <task-016>                |      63      |                                                          |
| [ ] <task-015>                |      62      |                                                          |
| [ ] <task-014>                |      61      |                                                          |
| [ ] <task-013>                |      60      |                                                          |
| [ ] <task-012>                |      59      |                                                          |
| [ ] <task-011>                |      58      |                                                          |
| [ ] <task-010>                |      57      |                                                          |
+-------------------------------+--------------+----------------------------------------------------------+
```

You would be able to sort by priority or by name

you would have a little ui at the bottom giving keybinds such movement keys (arrow keys/vim keys),
search the task by name (fuzzy finding: /), searching by tag (exact matching, comma-seperated: ?),
add a task (: a), edit a task (: e), remove a task (: d), enter detail mode (: <CR>), quit (: q),
tasks summary (: s).


Add task: opens a floating screen that prompts:

```txt
+-------------------------------------------------------------------------------------+
|                                                                                     |
|              +=================================================================+    |
| task name:   |                                                                 |    |
|              +=================================================================+    |
|                                                                                     |
|              +=================================================================+    |
|              |                                                                 |    |
| tags:        |                                                                 |    |
|              |                                                                 |    |
|              +=================================================================+    |
|                                                                                     |
|              +=================================================================+    |
| priority:    | 100                                                             |    |
|              +=================================================================+    |
|                                                                                     |
|              +=================================================================+    |
| related to:  | v  None                                                         |    |
|              +=================================================================+    |
|                                                                                     |
+-------------------------------------------------------------------------------------+
```

Remove task: prompts you a float screen asking if you really want to delete the task(s):

```txt
+-------------------------------------------------------+
|                                                       |
|  Do you really wish to delete the follow task(s):     |
|                                                       |
|    - task(     HUID0     ): <task-name>               |
|    - task(     HUID1     ): <task-name>               |
|    - task(     HUID2     ): <task-name>               |
|    - task(     HUID3     ): <task-name>               |
|                                                       |
|                                                       |
|          +=======+            +======+                |
|          | (Y)es |     or     | (N)o |                |
|          +=======+            +======+                |
|                                                       |
+-------------------------------------------------------+
```

Edit task opens the $EDITOR of choice, defaults to vim.

Finding by name or by tag reduces the amount of tasks displayed to the filtered ones

Detailed mode can be two different things:
  - either fully takes the space of the tui like:
```txt
+---------------------------------------------------------------------------------------------------------+
|                                                Details                                                  |
+---------------------------------------------------------------------------------------------------------+
| Crea | - Created: 2026-07-03 01:22:03                                                                   |
| <tas | - STATUS: OPEN                                                                                   |
| <tas | - TAGS: feature, cmdline-option, ui                                                              |
| <tas |                                                                                                  |
| <tas | Description:                                                                                     |
| <tas |                                                                                                  |
| <tas | Create a user-interface for the terminal using something like ncurses.                           |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
| <tas |                                                                                                  |
+-------------------------------+--------------+----------------------------------------------------------+
```
  - or opens in a floating window
