# the 'or' keyword doesn't seem to work anymore

- STATUS: CLOSED
- PRIORITY: 100
- TAGS: bug,filters

# Explaination
OR and AND operator were not correctly taken into account because they were considered like filtering by name


# Fix
Added two conditions to the name filtering: if string isn't "and" or "or" or "not"
