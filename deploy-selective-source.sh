#!/bin/bash
#   parameters
#   $1  Source repository URL
#   $2  URL of destination release repository
#   $3  User name
#   $4  Password
#   $5  Path to the Files/Folders to be deployed
#   $6  [--exclude <Pattern to exclude files/folders from deployment>]
#   $7  [[--commit <Deploy Branch Name> <Commit Message>] | [--tag <Release Tag Number>]]

if [ "$#" -lt 6 ]; then
    echo "Invalid number of arguments.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1
fi
if [ -z "$1" ]; then
    echo "Missing source repo URL.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1;
fi

if [ -z "$2" ]; then
    echo "Missing destination repo URL.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1;
fi
if [ -z "$3" ]; then
    echo "Missing username.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1;
fi
if [ -z "$4" ]; then
    echo "Missing password.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1;
fi
if [ -z "$5" ]; then
    echo "Missing File/Folder Path to Deploy.";
    echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
    exit -1;
fi
if [ "$#" -eq 6 ]; then
    if [ -z "$6" ]; then
        echo "Commit or Release options must be provided. Both are missing.";
		echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
        exit -1;
    fi
elif [ "$#" -gt 6 ]; then
    if [ -z "$6" ]; then
        echo "Argument Mismatch. Exclude path option is probably missing.";
		echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
        exit -1;
    fi
    if [ -z "$6" ]; then
        echo "Commit or Release options must be provided. Both are missing.";
		echo "usage: deploy-selective-source.sh <source-repo-url> <destination-repo-url> <user-name> <password> 
          <Path to the Files/Folders to be deployed> [--exclude  <Path to the Files/Folders to be excluded>] 
          [ [--commit <Deploy Branch Name> <Commit Message>]] | [--tag <Release Tag Number>] ]";
        exit -1;
    fi
fi


# Regex for checking whether the tags are SEMVER compliant
readonly SEMVER_REGEX="^(0|[1-9]\d*)+\.(0|[1-9]\d*)+\.(0|[1-9]\d*)+$"
readonly INTERNAL_SEMVER_REGEX="^(0|[1-9]\d*)+\.(0|[1-9]\d*)+\.(0|[1-9]\d*)+-rc(0|[1-9]\d*)+$"
readonly BRANCH_REGEX="^develop|master$"
readonly BITBUCKET_REGEX="bitbucket\.microchip\.com"

FILE_PATH=$5
IS_BITBUCKET_REPO=0

if [ ! -z "$1" ]; then	
	source_repo=($(echo $1))
fi

if [ ! -z "$2" ]; then	
	destination_repo=($(echo $2))
	if [[ "$destination_repo" =~ $BITBUCKET_REGEX ]];then
		IS_BITBUCKET_REPO=1
	fi
fi

if [ "$#" -eq 6 ]; then    
    commit_tag_info=($(echo $6))
elif [ "$#" -gt 6 ]; then
    exclude_pattern_argument=($(echo $6))
    if [ ${exclude_pattern_argument[0]} != "--exclude" ]; then
        echo "Incorrect/Mismatched Path exclude argument"
        echo "usage: \"--exclude <Path to the Files/Folders to be excluded>\"";
        exit -1;
    fi
    commit_tag_info=($(echo $7))
fi

if [ ${commit_tag_info[0]} == "--commit" ]; then
    if [ ${#commit_tag_info[@]} -lt 3 ]; then
        echo "Insufficient Commit arguments"
        echo "usage: \"--commit <Deploy Branch Name> <Commit Message>\"";
        exit -1;
    else 
        BRANCH=${commit_tag_info[1]};	
		COMMIT_MSG=$(echo ${commit_tag_info[@]:2} | sed "s/^[\"']\(.*\)[\"']$/\1/");		
		if [[ "$IS_BITBUCKET_REPO" == 0 ]];then 
			if [[ ! "$BRANCH" =~ $BRANCH_REGEX ]]; then
				echo "$BRANCH isn't a master or develop or does not match the branch pattern."
				exit -1
			fi
		fi
    fi
elif [ ${commit_tag_info[0]} == "--tag" ]; then
    if [ ${#commit_tag_info[@]} -lt 2 ]; then
        echo "Insufficient Tag arguments"
        echo "usage: \"--tag <Release Tag Number> <Tag Message>\"";
        exit -1;
    else
        TAG_NUMBER=${commit_tag_info[1]};
		if [[ "$IS_BITBUCKET_REPO" == 0 ]];then
			if [[ ! "$TAG_NUMBER" =~ $SEMVER_REGEX ]]; then
				echo "$TAG_NUMBER isn't a public release or does not match the SEMVER pattern."
				exit -1;
			fi 
		else 
			if [[ ( ! "$TAG_NUMBER" =~ $INTERNAL_SEMVER_REGEX ) && ( ! "$TAG_NUMBER" =~ $SEMVER_REGEX ) ]]; then
				echo "$TAG_NUMBER isn't a internal/public release or does not match the SEMVER pattern."
				exit -1;
			fi 
		fi
    fi
fi

TEMPDIR="temp"
cwd=$(pwd)
mkdir $TEMPDIR && cd $TEMPDIR && git clone --mirror "${source_repo}" .git
# Find out the tag branch
if [ ! -z $TAG_NUMBER ]; then
	git_commit_hash=$( git rev-list -n 1 $TAG_NUMBER)
	commit_hash_on_branches=$(git ls-remote origin | grep ${git_commit_hash} | grep 'refs/heads/')
	branch_name=$(echo ${commit_hash_on_branches%%$'\n'*} | cut -d' ' -f 2)
	TAG_BRANCH=${branch_name#"refs/heads/"}	
fi
# Find out the parent branch
if [ ! -z $BRANCH ]; then
	git config --unset core.bare
	git checkout $BRANCH
	PARENT_BRANCH=$( git show-branch  -a| sed "s/].*//" | grep "\*" | grep -v "$BRANCH" | head -n1 | sed "s/^.*\[//")
fi
cd "${cwd}" && rm -rf $TEMPDIR


# Embedding user-name and token in the destination url then check whether destination url is valid or not \
# The 'sed' tool is used with the regex '^https:\/\/\(.*\)\.git$' to find a match. If so, the string \
# is replaced with the captured inner group '.*' [Everything but 'https://' and '.git']
destRepoUrlCheck="https://$3:$4@$(echo "$2" | sed 's/^https:\/\/\(.*\)\.git$/\1/')"
destRepoUrlCheck=$destRepoUrlCheck

status=$(curl -s --head -w %{http_code} $destRepoUrlCheck -o /dev/null)

if [ $status != "501" ] && [ $status != "200" ]; then
    echo "Destination repository URL is either invalid or the repository is not available for public access.";
    exit -1;
fi


res=$(rsync --version)
if [[ -z $res ]]; then
	echo "rsync doesn't exist. Install rsync tool in your system."
	exit -1;
fi

# Do clean up before proceeding ,remove all the directories if any
WORKSPACE="deploy_sandbox"

# Delete the workspace if it already exists
rm -rf $WORKSPACE

echo "Starting deployment process at $(date)" ;
# Clone the Destination repository
git clone $destRepoUrlCheck $WORKSPACE

# Move in to the cloned workspace folder
cd $WORKSPACE

if [ ! -z $BRANCH ]; then
	#check branch exist in remote or local
	
	BRANCHES=$(git branch -a | sed -e 's/*/'$replace'/g')
	IS_BRANCH_EXIST=1
	if [[ ! "$BRANCHES" =~ $BRANCH ]]; then
		IS_BRANCH_EXIST=0
	fi
	
    # Ensure we checkout the required branch
	if [ $IS_BRANCH_EXIST -ne 0 ]; then	
		git checkout $BRANCH		
	else
		git checkout $PARENT_BRANCH
		git checkout -b $BRANCH
	fi
	
    # Wipe out the git directory (except the .git folder)
    rm -R *

    # Iterate through the ignore paths and form an exclude string for the rsync call
    if [ ! -z $exclude_pattern_argument ]; then
        for exclude_pattern in ${exclude_pattern_argument[@]:1}
        do
            EXCLUDE_STRING="$EXCLUDE_STRING--exclude="$exclude_pattern" "
        done
    fi

    # File paths need to be appended with ../ since we are working out of a workspace directory
    rsync -rd $EXCLUDE_STRING ../$(echo "$FILE_PATH" | sed 's/ / ..\//g') ../$WORKSPACE

    git add .

    if [ -z "$COMMIT_MSG" ]; then
        COMMIT_MSG="Merging into the ($BRANCH) branch"
    fi

    git commit -m "$COMMIT_MSG"

    # Push to remote origin(destination)
    echo "Pushing the $BRANCH branch to destination"
	if [ $IS_BRANCH_EXIST -ne 0 ]; then
		git push origin $BRANCH
	else
		git push -u origin $BRANCH
	fi
	
elif [ ! -z $TAG_NUMBER ]; then

	if [[ "$TAG_NUMBER" =~ $SEMVER_REGEX ]];then
	    # Ensure we checkout master - public tags will only be created on master
		git checkout master
	else
		# Ensure we checkout develop or feature branch - internal tags will only be created on develop or feature branch
		git checkout $TAG_BRANCH	
	fi
	
    git tag $TAG_NUMBER

    git push origin $TAG_NUMBER
fi

# Exit the script if git push fails
if [ "$?" != "0" ]; then
    echo "There was an error while deploying to destination repository. Please refer to the console log."
    exit 1
fi

echo "Ending the Deployment process at $(date)"

# Clean up the workspace
cd .. && rm -rf $WORKSPACE