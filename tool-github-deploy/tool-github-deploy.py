from github import Github
import sys
from src.cli import argsData
from src.release import release
from src.repo import repo
from src.deploy import deploy
from src.bitbucket import bitbucket

def main():
    try:
        args = argsData().argsGet()

        # Debug mode 
        if(args.debug_mode == True):
            print("\r\n[Debug] args:")
            print(args)
        
        if(args.github_personal_access_token or args.bitbucket_tag_operation):
            # Repository Deployment Operations
            if(args.deployment):
                githubAccess = Github(args.github_personal_access_token)
                deploy().performDeployOperations(args, githubAccess)
                print("[*] tool-github-deploy deployment operation done")
            
            # Repository Operations
            if(args.repo_operation or args.repo_update):
                githubAccess = Github(args.github_personal_access_token)
                repo().performRepoOperations(args, githubAccess)
                print("[*] tool-github-deploy repository operation done")
            
            # Repository Release Operations
            if(args.release_operation):
                githubAccess = Github(args.github_personal_access_token)
                release().performReleaseOperations(args, githubAccess)
                print("[*] tool-github-deploy release operation done")

            # Bitbucket Tag Operations
            if(args.bitbucket_tag_operation):
                bitbucket().performBitbucketOperations(args)
                print("[*] tool-github-deploy Bitbucket Tag operation done")

            sys.exit(0)      
        else:
            print("[!] Invalid github_personal_access_token key")
    except Exception as e:
        print(e)
        sys.exit(1)

if __name__ == "__main__":
    main()
    pass
