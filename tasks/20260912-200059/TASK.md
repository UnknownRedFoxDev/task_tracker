# change the way we write back to a task.md file

- STATUS: CLOSED
- PRIORITY: 100
- TAGS: 

# Currently
We are looking at a line and going back until a ':' has been hit.
Once hit, we increment by one to have the space.
Then we increment the file cursor one by one, reading the word until a space, comma or '\n' has been hit.
For every tag we need, if we're deleting tags, they are read, and if the read tag is the one to delete we pass it over and remove it from the sb.
For every tag we need, if we're adding a tag, the tag is added at the end of the line.

# New solution
Read until the line has been reached. Delete the whole line, and based on what line it is supposed to be, append the content + what's within the task.
This is much faster as we directly append the string rather than stepping one by one.
This is much better because we first modify the task's details, such as its status, tags, priority, and then we append based on those.

Sure, it's a bit annoying for tags if you delete/create a tag then render the line, and do that for each tag.
Though, at first, you could just do every transformation needed on the task itself. Then, render the line with the task content.

e.g.

task current state:
# test task

- STATUS: OPEN
- PRIORITY: 100
- TAGS: test, bogus, amongus

the command: `tatr overwrite -t -bogus,+tetanus -p 90 <huid>`
task next state:
# test task

- STATUS: OPEN

task next next state:
# test task

- STATUS: OPEN
- PRIORITY: 90

task next next next state:
# test task

- STATUS: OPEN
- PRIORITY: 90
- TAGS: amongus, test, tetanus

(-> the tag are reordered automaticaly)
