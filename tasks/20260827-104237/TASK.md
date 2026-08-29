# Create and parse a 'tags.md' file at the root inside tasks/

- STATUS: OPEN
- PRIORITY: 40
- TAGS: feature

This tags.md file should allow the tooling to give a descriptions for tags
i.e.:

tags.md:
```md
feature: implementing or updating a feature
cmdline-options: implementing a command line option like 'ls', 'summary', 'overwrite', ...
internal-rework: reworking internal structure, refactoring code on a larger scale
query-lang: doing modifications on the query language
bug: bug found in the code
testing: to testing features
optimize: feature or code that needs to be optimized
docs: where documentation needs to be done or updated
neovim: for my own neovim installation
```

Then for the summary cmdline-option:
```bash
$ tatr summary
[INFO] Summary of tasks:
OPEN:      4
CLOSED:   33
TOTAL:    37
UNTAGGED:  3
TAGGED:
            feature => 18  - Implementing or updated a feature
    cmdline-options => 15  - Implementing a command line option like 'ls', 'summary', 'overwrite', ...
    internal-rework => 13  - Reworking internal structure, refactoring code on a larger scale
         query-lang => 10  - Doing modifications on the query language
                bug => 5   - Bug found in the code
            testing => 3   - To test features
           optimize => 1   - Feature or code that needs to be optimized
               docs => 1   - Where documentation needs to be done or updated
             neovim => 1   - For my own neovim installation
```

Though, if some are not covered two cases can arise:

1. The tooling crashes
2. The tooling ignores the NULL that it would return

To which the summary would look a bit weird:

tags.md:
```md
aboba: awjklnasemkfsdvhjasnmkdcnk
ccc: asdjkljkl;qwqiozxcmkldvnmk
bbbbbbbbb: asdjklasdjkljkl;asdfsdfjkl;
```

```bash
$ tatr summary
TAGGED:
       aboba => 69  - awjklnasemkfsdvhjasnmkdcnk
       aaaaa => 42
         ccc => 7   - asdjkljkl;qwqiozxcmkldvnmk
   bbbbbbbbb => 6   - asdjklasdjkljkl;asdfsdfjkl;
```
