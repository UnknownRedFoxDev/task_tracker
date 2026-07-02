# Create a new cmdline option 'init' to initialize a 'tasks/' directory if it does not exists

- STATUS: CLOSED
- PRIORITY: 70
- TAGS: feature,cmdline-options

As of right now, the tool only relies on the end-user to already have a directory named "tasks" in the project root.

However, one could want to initialize their repository by creating a tasks direcotry. Sure, it's for the lazy people, but at least the name of the directory is now enforced to be "tasks".

Consider that by task(20260701-200323), the tool should first try to find the "tasks" directory, recursively 2 levels down, **but** also up.

If the directory is not found, create one where the command is invoked, aka, the current working directory (cwd for short).

Although, if the "tasks" directory is found, it should prompt you for confirmation that you want to create a tasks directory to where you are.
In the prompt, it should also tell the user that a tasks folder already exists wherever from the cwd by giving its path.
