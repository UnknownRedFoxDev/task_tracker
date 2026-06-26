# Fix issue where listing needs double quoting between tags string in order to have more than one tag filtering

- STATUS: CLOSED
- PRIORITY: 100
- TAGS: cmdline-options,internal-rework,bug

Rather than doing:

tatr ls ".CLOSED and .feature"

do:

tatr ls .CLOSED and .feature
