# Recursive search for the tasks/ directory should try to search up from at worse 2 directories

- STATUS: CLOSED
- PRIORITY: 80
- TAGS: internal-rework

From wherever you are in your project directory, if you are in src/, src/<inner-dir>/, assets/, assets/<inner-dir>/, `tatr` should still be able to find the tasks/ directory at the project's root.

# Process

## From cwd
It searches at cwd every directory available, apart from those starting with a '.' the likes of `.`, `..`, `.git`, `.idea`, etc, for one that is named "tasks".

### If none found
It descends down once into every directory inside cwd that do not start with a dot. It then it searches every directory for the "tasks" directory.

If successful, it stops and returns the path to it. Otherwise, it returns `NULL` rendering an error.

## What should be done now
As of right now, after the first descend, if not found, it stops and errors out.
But, what should be done is a upward descend. Basically, go up once, for every directory there, expect the one you came from, and search from those directories.

It changes the cwd to those new directories and do the initial directory and then descend again if still not found.

If still not found after those directories, go up once more and do the same search.
