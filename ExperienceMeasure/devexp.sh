#!/usr/bin/env bash

UNIX_DAY=86400
RETVAL=-1
FILE_NAME="experience.txt"

#source: https://unix.stackexchange.com/a/218522
inside_git_repo()
{
    inside_git_repo="$(git rev-parse --is-inside-work-tree 2>/dev/null)"

	if [ "$inside_git_repo" ]; then
	  echo "inside git repo"
	  RETVAL=1
	else
	  echo "not in git repo"
	  RETVAL=0
	fi
}

calculate_experience()
{
	inside_git_repo
	if [ $RETVAL == 0 ] 
	then 
		return
	fi

	# The length of a day in a Unix timestamp
	REPO_NAME=$(basename -s .git `git config --get remote.origin.url`)
	echo -------- $REPO_NAME ---------

	# Create temporary files to contain data to iterate over
	# File is removed when quitting or crashing
	daysTmp=$(mktemp)
	exec {FD_W}>"$daysTmp"	# Opens filedescriptors
	exec {FD_R}<"$daysTmp"
	rm "$daysTmp"

	authorsTmp=$(mktemp)
	exec {FD_W}>"$authorsTmp"	# Opens filedescriptors
	exec {FD_R}<"$authorsTmp"
	rm "$authorsTmp"

	# Get all author names from commits and save to file
	git shortlog -snc | sed 's/^\s*[0-9]*\s*//g' > $authorsTmp

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
			echo $IN_DAYS >> $daysTmp
		fi
	done < $authorsTmp

	# Sums the days and divides it by the amount of considered committers.
	TOTAL=$(awk '{ sum += $1 } END { print sum }' $daysTmp)
	LINES=$(wc -l $daysTmp | awk '{ print $1 }')
	MEAN=$(expr $TOTAL / $LINES)

	# Prints values
	echo "Total:" $TOTAL
	echo "Lines:" $LINES
	echo "Mean:" $MEAN

	echo "${REPO_NAME},${LINES},${TOTAL},${MEAN}" >> ../${FILE_NAME}
}

echo repository,authors_considered,total_experience,mean_experience > ${FILE_NAME}

for f in ./*/; do
    if [ -d ${f} ]; then  
    	(cd "$f" && calculate_experience);
    fi
done
