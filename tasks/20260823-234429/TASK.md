# Optimize NODE_AND eval_node() case when accumulating both results array

- STATUS: OPEN
- PRIORITY: 50
- TAGS: optimize,internal-rework

change the result to be a dynamic array such that:

1. you can just reset the array by doing: array.count = 0, which is notably much faster than memsetting it to 0
2. you can use `remove_unordered(&arr, i)` on both arrays, while also reducing `i` and `lhs_ite` by 1.
