#!/bin/bash
#   parameters
#   $1  URL of Bitbucket repository to be cloned(source)
#   $2  URL of github release repository (destination)
#   $3  Github User name
#   $4  Github Password
#   $5  [--branch <develop> <master>]
#   $6  [--tag <release-tag-number> or --tag <public> or --tag <internal> or --tag <all> or --tag -r <regex>]
if [ "$#" -lt 4 ]; then
    echo "Invalid number of arguments.";
    echo "usage: deploy-source-as-is.sh <bitbucket-repo-url> <github-repo-url> <github-user-name> <github-password> [--tag <release-tag-number> | <public> | <internal> | <all> | -r <regex>] [--branch develop]";
    exit -1
fi
if test -z "$1"; then
    echo "Missing Bitbucket URL.";
    echo "usage: deploy-source-as-is.sh <bitbucket-repo-url> <github-repo-url> <github-user-name> <github-password> [--tag <release-tag-number> | <public> | <internal> | <all> | -r <regex>] [--branch develop]";
    exit -1;
fi
if test -z "$2"; then
    echo "Missing Github URL.";
    echo "usage: deploy-source-as-is.sh <bitbucket-repo-url> <github-repo-url> <github-user-name> <github-password> [--tag <release-tag-number> | <public> | <internal> | <all> | -r <regex>] [--branch develop]";
    exit -1;
fi
if test -z "$3"; then
    echo "Missing Github username.";
    echo "usage: deploy-source-as-is.sh <bitbucket-repo-url> <github-repo-url> <github-user-name> <github-password> [--tag <release-tag-number> | <public> | <internal> | <all> | -r <regex>] [--branch develop]";
    exit -1;
fi
if test -z "$4"; then
    echo "Missing Github password.";
    echo "usage: deploy-source-as-is.sh <bitbucket-repo-url> <github-repo-url> <github-user-name> <github-password> [--tag <release-tag-number> | <public> | <internal> | <all> | -r <regex>] [--branch develop]";
    exit -1;
fi

#Variables
githubUrl=$2
githubUserName=$3
githubPassword=$4
readonly HTTPS="https://"
#Regex for checking whether the tags are SEMVER compliant
readonly SEMVER_REGEX="^(0|[1-9]\d*)+\.(0|[1-9]\d*)+\.(0|[1-9]\d*)+$"
readonly DEVELOP_BRANCH="develop"
readonly MASTER_BRANCH="master"
readonly INTERNAL_RELEASE_TAGS="internal"
readonly PUBLIC_RELEASE_TAGS="public"
readonly ALL_TAGS="all"
readonly RC_TAG="rc"


#Optional parameter "--branch" and "--tag" validation 
for (( argIndex=5; argIndex<=$#; argIndex++ ))
do
    branchesOrTags=($(echo ${!argIndex}))
    if test ! -z "$branchesOrTags"; then
        if [ ${branchesOrTags[0]} == "--branch" ]; then
            if [ ${#branchesOrTags[@]} -lt 1 ]; then
                echo "Insufficient branch arguments"
                echo "usage: \"--branch <develop>\"";
                exit -1;
            else
		    	branches=("${branchesOrTags[@]:1}")
				branchCount=($(echo ${#branches[@]}))
		    	for (( branchIndex=0; branchIndex<$branchCount; branchIndex++ ))
			    do
                    if [[ ! ${branches[branchIndex]} =~ $DEVELOP_BRANCH && ! ${branches[branchIndex]} =~ $MASTER_BRANCH ]]; then
                        echo "Only develop or master branch can be deployed."
                        exit -1;
                    fi 
                done				
            fi
        elif [ ${branchesOrTags[0]} == "--tag" ]; then
            if [ ${#branchesOrTags[@]} -lt 1 ]; then
                echo "Insufficient Tag arguments"
                echo "usage: \"--tag <release-tag-number>\" or \"--tag internal\" or \"--tag public\" or \"--tag all\" or \"--tag -r regex\"";
                exit -1;
            else
			    tagOptions=("${branchesOrTags[@]:1}")
				tagoptionCount=($(echo ${#tagOptions[@]}))
				if [ $tagoptionCount == 1 ]; then
                    TAG_INFO=${tagOptions[0]};
                    if [[ ! "$TAG_INFO" =~ $SEMVER_REGEX &&  ! "$TAG_INFO" =~ $INTERNAL_RELEASE_TAGS &&   ! "$TAG_INFO" =~ $PUBLIC_RELEASE_TAGS  && ! "$TAG_INFO" =~ $ALL_TAGS ]]; then
                        echo "$TAG_INFO isn't a public release or does not match the SEMVER pattern or tag option is not the supported "
                        exit -1;
                    fi  
                elif [ $tagoptionCount == 2 ]; then
                    TAG_INFO=${tagOptions[1]};
                else
				    echo "Invalid Tag arguments"
                    echo "usage: \"--tag <release-tag-number>\" or \"--tag internal\" or \"--tag public\" or \"--tag all\" or \"--tag -r regex\"";
                fi				
              				
            fi
        fi
    fi
done

#if "--branch" argument is not supplied then by default deploy only master branch
if test -z "$branches"; then
    branches=($(echo "master"))
fi

#Check bitbucket url is valid.currently our bitbucket server does not support the functionality required to fulfill the request(return HTTP code 501).
bitbucketUrl=$1
bitbucketUrlCheck=$bitbucketUrl
if [[ $bitbucketUrl =~ ".git" ]]; then
    bitbucketUrlCheck=$(echo ${bitbucketUrl%.git})
fi
status=($(echo $(git ls-remote $bitbucketUrlCheck)))
if test -z $status; then
 echo "Bitbucket repository URL is either invalid or the repository is not available for public access.";
 exit -1;
fi

#Embedding user-name and token in the github url then check whether github url is valid or not
userNamePassword=$githubUserName":"$githubPassword
gitbubUrlSecondPart="${githubUrl#'https://'}"
githubUrl="${HTTPS}${userNamePassword}@${gitbubUrlSecondPart}"
githubUrlCheck=$githubUrl
if [[ $githubUrl =~ ".git" ]]; then
    githubUrlCheck=$(echo ${githubUrl%.git})
fi
status=$(curl -s --head -w %{http_code} $githubUrlCheck -o /dev/null)
if [ $status != "200" ]; then
    echo "Github repository URL is either invalid or the repository is not available for public access.";
    exit -1;
fi


#extract the git repository name
repoName=$(echo ${githubUrl%.git})
temp=$(echo "${repoName%/*}")
repoName=$(echo $repoName | sed 's|.*/||')
userName=$(echo $temp | sed 's|.*/||')
repoName="${userName}/${repoName}"



#Do clean up before proceeding ,remove all the directories if any
WORKSPACE="deploy_sandbox"
#delete the workspace if it already exists
rm -rf $WORKSPACE

echo "Starting deployment process at $(date)" ;
#Clone the repository
git clone $bitbucketUrl $WORKSPACE

# go to cloned project folder - workspace folder
cd $WORKSPACE

checkoutBranch()
{
    # ensure we checkout the given branch
    git checkout --track origin/$1

    git remote set-url origin $githubUrl
}

publishBranch ()
{
    # push to remote origin(github)
    echo "Pushing the $1 branch to Github"
    git push origin $1

    #Exit the script if git push is failed.
    if [ "$?" != "0" ]; then
        echo "There was an error while pushing the $1 branch to Github. Please refer the logs."
        exit -1
    fi
}

publishAllPublicReleaseTags()
{
    #Push all the public release tags to Github
    arr=($(echo $(git tag)))
    for tag in "${arr[@]}"
    do
        if [[ "$tag" =~ $SEMVER_REGEX ]]; then
            tagPresent=$(git ls-remote origin refs/tags/$tag)
            if test -z "$tagPresent"; then
                git push origin $tag
			    echo "pushing tag $tag to remote Github repo"
		    else
		        echo "Tag: $tag is already present in the remote Github repo"
	        fi
        else
            echo "$tag isn't a public release or does not match the SEMVER pattern."
       fi
    done
}

publishInternalReleaseTags()
{
    #Push all the public release tags to Github
    arr=($(echo $(git tag)))
    for tag in "${arr[@]}"
    do
	    releaseTag=${tag,,}
        if [[ "$releaseTag" == *"$RC_TAG"* ]]; then
            tagPresent=$(git ls-remote origin refs/tags/$tag)
            if test -z "$tagPresent"; then
                git push origin $tag
			    echo "pushing tag $tag to remote Github repo"
		    else
		        echo "Tag: $tag is already present in the remote Github repo"
	        fi
        else
            echo "$tag isn't a internal release tag."
       fi
    done
}

publishMatchedTags()
{
    #Push all the public release tags to Github
    arr=($(echo $(git tag)))
    for tag in "${arr[@]}"
    do
        if [[ "$tag" =~ $1 ]]; then
            tagPresent=$(git ls-remote origin refs/tags/$tag)
            if test -z "$tagPresent"; then
                git push origin $tag
			    echo "pushing tag $tag to remote Github repo"
		    else
		        echo "Tag: $tag is already present in the remote Github repo"
	        fi
        else
            echo "$tag doesn't match the specified regex."
       fi
    done
}

publishAllTags()
{
    #Push all the tags to Github
    arr=($(echo $(git tag)))
    for tag in "${arr[@]}"
    do
        tagPresent=$(git ls-remote origin refs/tags/$tag)
        if test -z "$tagPresent"; then
            git push origin $tag
		    echo "pushing tag $tag to remote Github repo"
	    else
	        echo "Tag: $tag is already present in the remote Github repo"
	    fi
    done
}

publishTheGivenTag ()
{
    localTagPresent=$(git tag -l "$TAG_INFO")
	if test ! -z "$localTagPresent"; then
        remoteTagPresent=$(git ls-remote origin refs/tags/$TAG_INFO)
        if test -z "$remoteTagPresent"; then
		    echo "pushing tag $TAG_INFO to remote Github repo"	
			response=$(curl -s -H "Authorization: token $githubPassword" -d '{"tag_name":"'"$TAG_INFO"'", "name":"'"$TAG_INFO"'","body":"'"$TAG_INFO"'"}' "https://api.github.com/repos/$repoName/releases")
		else
		    echo "Tag: $tag is already present in the remote Github repo"
	    fi
	else
	    echo "Tag: $tag is not present in the cloned bitbucket repository"
		exit -1
	fi
}

publishTags()
{
    if test ! -z "$TAG_INFO"
	then
	    if [[ "$TAG_INFO" =~ $SEMVER_REGEX ]] 
		then
            publishTheGivenTag
		elif [[ "$TAG_INFO" =~ $INTERNAL_RELEASE_TAGS ]]
		then
		    publishInternalReleaseTags
		elif [[ "$TAG_INFO" =~ $PUBLIC_RELEASE_TAGS ]]
		then
		    publishAllPublicReleaseTags
		elif [[ "$TAG_INFO" =~ $ALL_TAGS ]]
		then
		    publishAllTags
		else
		    publishMatchedTags $TAG_INFO
		fi
    else
        publishAllPublicReleaseTags
    fi
}

branchCount=($(echo ${#branches[@]}))
for (( i=0; i<$branchCount; i++ ))
do
    checkoutBranch  ${branches[i]}
    publishBranch  ${branches[i]}
done

#Publish the tags
publishTags

echo "Ending deployment process at $(date)"

#Cleans up the workspace
cd .. && rm -rf $WORKSPACE
