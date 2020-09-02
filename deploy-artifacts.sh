#!/bin/bash
#   parameters
#   $1  URL of github release repository (destination)
#   $2  Github user-name
#   $3  Github password
#   $4  Release Commit Message/Release Tag Name
#   $5  Release name
#   $6  Release description
#   $7  First artifact , user can specify multiple artifacts as arguments
#
#   Assuming Github repository is already created and its not empty
#
if [ "$#" -lt 7 ]; then
    echo "Invalid number of arguments.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1
fi
if test -z "$1"; then
    echo "Missing Github URL.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$2"; then
    echo "Missing user-name.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$3"; then
    echo "Missing password.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$4"; then
    echo "Missing release tag name.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$5"; then
    echo "Missing release name.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$6"; then
    echo "Missing release description.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi
if test -z "$7"; then
    echo "Missing release artifacts.";
    echo "usage: deploy-artifacts.sh <github-repo-url> <github-user-name> <github-password> <release-tag-name> <release-name> <release-description> <artifact1> <artifact2> ...";
    exit -1;
fi

githubUrl=$1
githubUserName=$2
githubPassword=$3
tagName=$4
releaseName=$5
releaseDescription=$6
readonly ARTIFACTNAME_ARG_START_INDEX=7

# Embedding user-name and token in the github url then check whether github url is valid or not \
# The 'sed' tool is used with the regex '^https:\/\/\(.*\)\.git$' to find a match. If so, the string \
# is replaced with the captured inner group '.*' [Everything but 'https://' and '.git']
githubUrlCheck="https://$githubUserName:$githubPassword@$(echo "$githubUrl" | sed 's/^https:\/\/\(.*\)\.git$/\1/')"
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

response=$(curl -s -H "Authorization: token $githubPassword" -d '{"tag_name":"'"$tagName"'", "name":"'"$releaseName"'","body":"'"$releaseDescription"'"}' "https://api.github.com/repos/$repoName/releases")
# retain the part after the "upload_url" string
temp=${response##*\"upload_url\": \"} 
# Extract the upload_url
upload_url=${temp%\{\?name,label\}\",*}  
upload_url="${upload_url%\{*}"
#make sure there is no error , if upload url is retrieved then continue else exit the script
if [[ ! "$upload_url" =~ "https" ]]; 
then
    exit -1;
fi

echo "uploading release asset to url : $upload_url"
index=$ARTIFACTNAME_ARG_START_INDEX
while [ "$index" -le "$#" ]; 
do
    eval "arg=\${$index}"
    artifactName=$arg
    if test ! -z "$artifactName"; then
        type=$(echo "${artifactName##*.}")
        labelName=$(basename $artifactName)
        finalResponse=$(curl -s -H "Authorization: token $githubPassword"  \
          -H "Content-Type: application/$type" \
          --data-binary @$artifactName  \
          "$upload_url?name=$labelName&label=$labelName")
		#Retrieve the download url
		download_url=${finalResponse#*\"browser_download_url\":\"}
		download_url=${download_url%\"*}   
		#If the response has the download url for the artifact then deployment is successful
		if [[ ! "$download_url" =~ "$labelName" ]]; 
        then
		    echo "Artifact deployment failed. Make sure artifact is available in the specified location and Github repository is accessible."
            exit -1;
        fi
    fi
    index=$((index + 1))
done