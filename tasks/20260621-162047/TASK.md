# Find cwd and search recursively from it the 'tasks/' directory.

- STATUS: OPEN
- PRIORITY: 1
- TAGS:

Tasks directory should always be at the cwd. Aka, if at at a project's root, you should have:

project's root
     |
     |
     +--- build/
     |
     +--- src/
     |
     +--- tasks/
     |
     ...

If you have a weird layout where you put your tasks folder somewhere else, just execute tatr from the place where 'tasks/' is
