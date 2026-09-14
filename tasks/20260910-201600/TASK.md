# Change overwrite argument order to take task huid as the last argument to allow multiple arguments

- STATUS: CLOSED
- PRIORITY: 60
- TAGS: internal-rework

The argument parser show create a list of tasks, validate those that actually exist from those that don't (i.e. delete invalid ones), and give that list to the overwrite fn.

