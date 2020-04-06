#!/bin/bash

# The length of a day in a Unix timestamp
UNIX_DAY=86400

# Get all author names from commits and save to file
AUTHORS=$(git shortlog -snc | sed 's/^\s*[0-9]*\s*//g' > authors.txt)

# TODO: Remove this and replace all files with temporary files
if test -f "days.txt"
then
	> days.txt
fi

# For each committer from the file,
# calculate the days between their first and last commit to the repository.
while read p; do
	# TODO: Check if author has made just a single commit (last and first are equal).
	FIRSTCOMMIT=$(git log --format=%ct --author="$p" --reverse | head -n 1)
	LASTCOMMIT=$(git log --format=%ct --author="$p" | head -n 1)
	if [ -z $FIRSTCOMMIT ] # If it is null, no commits have been made.
	then
		continue
	fi	
	DIFFERENCE=$(expr $LASTCOMMIT - $FIRSTCOMMIT)

	# Checks if the committer has made at least 
	# two commits with at least a days difference.
	if [ $DIFFERENCE != 0 -a $UNIX_DAY -le $DIFFERENCE ] 
	then
		IN_DAYS=$(expr $DIFFERENCE / $UNIX_DAY)
		echo "$p: $IN_DAYS"
		# Write the amount of days to file, these will be summed later.
		echo $IN_DAYS >> days.txt
	fi
done < authors.txt

# Sums the days and divides it by the amount of considered committers.
TOTAL=$(awk '{ sum += $1 } END { print sum }' days.txt)
LINES=$(wc -l days.txt | awk '{ print $1 }')
MEAN=$(expr $TOTAL / $LINES)

# Prints values
echo "Total:" $TOTAL
echo "Lines:" $LINES
echo "Mean:" $MEAN