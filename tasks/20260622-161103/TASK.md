# Optimize space usage when filtering by tags

- STATUS: OPEN
- PRIORITY: 1
- TAGS:

# Currently
we have:

a result array of task_t *
a temporary array of arrays of task_t *
a HTable of key: uuid (char *), val: task_t *

## NONE and NOT
When either NONE or NOT mode are enabled, we allocate a new array inside of the temporary one then, for each task that fits the tag, we add it to the array.
Once done, we add it all to the result array and send it off to be printed.

## AND
(side thought, we can directly put into the HTable the 'getting-computed-task-array' if we look for that each task answers both `tag1` and `tag2`)
When AND mode is enabled, we take the already computed previous task array inside the temporary array of arrays, and compute a new array of tasks that answer to the next tag.

Once that's done, we put into the HTable the new computed array if each of the task in there answer to the previous tag.
Then, in the HTable again, the previous task array if each tasks inside answer to the next tag and if the task isn't already inside.

## OR
When OR mode is enabled, we compute and put directly into the HTable every task that answers to the next tag.
Then, we go through the previous task array and put it into the HTable if the task doesn't already exists.

### AND and OR
Once the HTable is done getting filled up, we reset the newly computed tasks array, we then put into the result array every task inside the HTable, and then we put back where the previously newly computed tasks array was, the result array and continue the parsing.

## Once done
if result array is empty (NONE, or NOT) we fill it up with the temporary array's last array computed.
we then free every slot used of the temporary array and itself too.
result is finally returned.

# What can be done
we have:

a result array of task_t *
a second array of task_t * (only for AND mode)
a HTable of uuid->task_t *

## NONE and NOT
we directly fill the result array.

## AND
previous task array is in the result array.
we compute the next task array into the second array

we put the previous task array's tasks inside the HTable if they answers to the next tag
we put the next task array's tasks inside the HTable if they answers to the previous tag

we are assured of no duplicate with ht_find_or_put

then we put everything back into the result array and reset the second array.

## OR
we put the previous task array into the HTable
we compute the next task array directly into the HTable

then we put everything back into the result array.
