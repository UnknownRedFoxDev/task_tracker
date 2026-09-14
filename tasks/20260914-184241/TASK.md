# Make a more robust path finder to get the closest tasks/ directory

- STATUS: OPEN
- PRIORITY: 100
- TAGS: internal-rework

# Currently
The system to find the tasks/ directory works and the solution is "fine".
However, I think I should try to have a more robust solution.

Idea: look at how Tsoding made his tool find his tasks/ directory

Moreover, in a Tsoding stream, at some point there was a program to find the relative and the absolute path from a current path to the target path.
This is could be an interesting starting point.

To find the target path by either going up directories, paths. Or go down recurvise decent style. :cool:
