# Optimize NODE_AND eval_node() case when accumulating both results array

- STATUS: CLOSED
- PRIORITY: 50
- TAGS: optimize,internal-rework,unresolved

change the result to be a dynamic array such that:

1. you can just reset the array by doing: array.count = 0, which is notably much faster than memsetting it to 0
2. you can use `remove_unordered(&arr, i)` on both arrays, while also reducing `i` and `lhs_ite` by 1.

This is a bigger change than what was thought of. this task is about replacing the result array by a dynamic arr entirely. which granted, reading back the task, it is already said, but this encompasses the eval_node, print_tasks and the sorting of the tasks.

A way to do this is, could be to replace result by a `tasks_t` array.

---

# Retrospective
This task exploded in complexity due to the need to change `tasks_t` to hold an array of pointers to `task_t` (aka `task_t **`).
because of that, the whole task.c file has to account for that change. not just the eval_tag() function.

So for right now, unless a better idea is found, this task is set as CLOSED, but unresolved.
