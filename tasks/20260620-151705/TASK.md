# ls flag should become a string flag that filters the tasks by the tag given

- STATUS: CLOSED
- PRIORITY: 69
- TAGS: internal-rework,cmdline-options,filters

If the string given is empty/missing, print all tasks
If the string given corresponds to the status, either CLOSED or OPEN, filter by such
If the string given does not correspond to a task's status, filter by the tags
