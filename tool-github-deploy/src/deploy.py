from src.utils import utils
from src.repo import repo
from datetime import datetime
import subprocess
from subprocess import Popen, PIPE, check_call, STDOUT
import os
import re

class deploy:
    """
    This class represents execute deployment operations.
    """

    workSpaceFolder = "deploy_sandbox"

    def cloneRepository(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        if(args.deploy_bitbucket_url and not args.deploy_bitbucket_repo_folder):
            # Clone Git repository
            pOpenCommandList = ["git", "clone",  args.deploy_bitbucket_url, self.workSpaceFolder]
            utils().popenTask(pOpenCommandList)
        else:
            # Copy Git repository
            utils().copytree(args.deploy_bitbucket_repo_folder, self.workSpaceFolder)
        pass 

    def createWorkSpace(self):
        """
        :type: None
        """
        utils().makedirs(self.workSpaceFolder)
        pass

    def excludeFiles(self, args, branch):
        """
        :param args: command line input arguments
        :param branch: git branch name
        :type: None
        """

        if(args.deploy_excludes_files):
            print("[*] Excluding files...")
            excludeFileList = args.deploy_excludes_files.split(',')

            for excludeFile in excludeFileList:
                excludeFile = excludeFile.replace(" ", "")
                utils().purge('.', excludeFile)

            if(args.debug_mode):        
                pOpenCommandList = ["git", "status"]
                status = utils().popenTask(pOpenCommandList)
                print("[ Debug mode ] Git status: " + str(status))

            pOpenCommandList = ["git", "add", "-A"]
            status = utils().popenTask(pOpenCommandList)
        
            if(args.deploy_excludes_files_commit):
                pOpenCommandList = ["git", "commit", "-m", args.deploy_excludes_files_commit]
                status = utils().popenTask(pOpenCommandList)
            else:
                pOpenCommandList = ["git", "commit", "-m", "Merging into the " + branch + " branch"]
                status = utils().popenTask(pOpenCommandList)

            if(args.debug_mode):        
                pOpenCommandList = ["git", "status"]
                status = utils().popenTask(pOpenCommandList)
                print("[ Debug mode ] Git status: " + str(status))

    def excludeFilesForTag(self, args, tagName):
        """
        :param args: command line input arguments
        :param branch: git tag name
        :type: None
        """

        if(args.deploy_excludes_files):
            print("[*] Excluding files in tag: " + tagName)

            #git branch 1.0.0-branch 1.0.0
            pOpenCommandList = ["git", "branch", tagName + "-branch", tagName]
            status = utils().popenTask(pOpenCommandList)

            #git checkout 1.0.0-branch
            pOpenCommandList = ["git", "checkout", tagName + "-branch"]
            status = utils().popenTask(pOpenCommandList)
            
            excludeFileList = args.deploy_excludes_files.split(',')

            for excludeFile in excludeFileList:
                excludeFile = excludeFile.replace(" ", "")
                utils().purge('.', excludeFile)

            if(args.debug_mode):        
                pOpenCommandList = ["git", "status"]
                status = utils().popenTask(pOpenCommandList)
                print("[ Debug mode ] Git status: " + str(status))

            pOpenCommandList = ["git", "add", "-A"]
            status = utils().popenTask(pOpenCommandList)
        
            if(args.deploy_excludes_files_commit):
                pOpenCommandList = ["git", "commit", "-m", args.deploy_excludes_files_commit]
                status = utils().popenTask(pOpenCommandList)
            else:
                pOpenCommandList = ["git", "commit", "-m", tagName]
                status = utils().popenTask(pOpenCommandList)

            if(args.debug_mode):        
                pOpenCommandList = ["git", "status"]
                status = utils().popenTask(pOpenCommandList)
                print("[ Debug mode ] Git status: " + str(status))

            #git tag -d 1.0.0
            pOpenCommandList = ["git", "tag", "-d", tagName]
            status = utils().popenTask(pOpenCommandList)
            
            #git tag 1.0.0
            pOpenCommandList = ["git", "tag", tagName]
            status = utils().popenTask(pOpenCommandList)

            try:
                #git tag 1.0.0
                pOpenCommandList = ["git", "push", "origin", ":" + tagName]
                status = utils().popenTask(pOpenCommandList)
                pass
            except Exception as identifier:
                pass

    def checkoutBranch(self, args, branch):
        """
        :param args: command line input arguments
        :param branch: Bitbucket branch name
        :type: None
        """

        # ensure we checkout the given branch
        pOpenCommandList = ["git", "checkout", "--track", "origin/" + branch]
        status = utils().popenTask(pOpenCommandList)

        error = status[1]

        if(error.find("already exists") > -1):
            pOpenCommandList = ["git", "checkout", branch]
            utils().popenTask(pOpenCommandList)

    
        pOpenCommandList = ["git", "remote", "set-url", "origin", args.deploy_github_url]
        utils().popenTask(pOpenCommandList)


    def publishBranch(self, args, branch):
        """
        :param branch: Bitbucket branch name
        :type: None
        """

        if(args.debug_mode):
            print("[Debug] " + branch)
            pOpenCommandList = ["git", "branch"]
            status = utils().popenTask(pOpenCommandList)
            print("[Debug] " + str(status))

        pOpenCommandList = ["git", "pull", "origin", branch]
        utils().popenTask(pOpenCommandList)

        if(args.deploy_excludes_files):
			# Update git user information
            pOpenCommandList = ["git", "config", "--global", "user.email", "'microchip@microchip.com'"]
            utils().popenTask(pOpenCommandList)
            pOpenCommandList = ["git", "config", "--global", "user.name", "'Microchip Technology'"]
            utils().popenTask(pOpenCommandList)
			
            self.excludeFiles(args, branch)
            pOpenCommandList = ["git", "pull", "origin", branch]
            utils().popenTask(pOpenCommandList)


        # push to remote origin(github)
        print("[*] Pushing the " + branch + " branch to Github")

        pOpenCommandList = ["git", "push", "origin", branch]
        status = utils().popenTask(pOpenCommandList)
        error = status[1]

        # Exit the script if git push is failed.
        if(error.find("error") > -1):
            if(args.debug_mode):
                print("[Debug] " + str(error))
                print("[Debug] There was an error while pushing the "+ branch +" branch to Github. Please refer the logs.")

            utils().githubExceptionErrorMessage(error)
                

    def publishAllPublicReleaseTags(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        # Push all the public release tags to Github
        pOpenCommandList = ["git", "tag"]
        status = utils().popenTask(pOpenCommandList)
        output = status[0]
        output = output.split('\n')
        tagList = utils().removeSpaceAndSpecialCharFromList(output)

        for tag in tagList:
            if(utils().checkSemanticVersioningRegex(tag)):
                pOpenCommandList = ["git", "ls-remote", "origin", "refs/tags/" + tag]
                status = utils().popenTask(pOpenCommandList)
                output = status[0]

                self.excludeFilesForTag(args, tag)

                if(not output):
                    pOpenCommandList = ["git", "push", "origin", tag]
                    utils().popenTask(pOpenCommandList)
                    print("[*] pushing tag " + tag + " to remote Github repo")
                else:
                    print("[-] Tag: " + tag + " is already present in the remote Github repo")

            else:
                print("[-] " + tag + " isn't a public release or does not match the SEMVER pattern.")


    def publishInternalReleaseTags(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        # Push all the public release tags to Github
        pOpenCommandList = ["git", "tag"]
        status = utils().popenTask(pOpenCommandList)
        output = status[0]
        output = output.split('\n')
        tagList = utils().removeSpaceAndSpecialCharFromList(output)

        for tag in tagList:
            if(tag.upper().find("RC") > -1):
                pOpenCommandList = ["git", "ls-remote", "origin", "refs/tags/" + tag]
                status = utils().popenTask(pOpenCommandList)
                output = status[0]
                
                self.excludeFilesForTag(args, tag)

                if(not output):
                    pOpenCommandList = ["git", "push", "origin", tag]
                    status = utils().popenTask(pOpenCommandList)
                    output = status[0]
                    print("[*] pushing tag " + tag + " to remote Github repo")
                else:
                    print("[-] Tag: " + tag + " is already present in the remote Github repo")
            else:
                print("[-] Tag:" + tag + " isn't a internal release tag.")

    def publishRegexMatchedTags(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        # Push all the public release tags to Github
        pOpenCommandList = ["git", "tag"]
        status = utils().popenTask(pOpenCommandList)
        output = status[0]
        output = output.split('\n')
        tagList = utils().removeSpaceAndSpecialCharFromList(output)

        if(args.deploy_bitbucket_tag[0] == '*'):
            pattern = re.compile("."+ args.deploy_bitbucket_tag)
        else:
            pattern = re.compile(args.deploy_bitbucket_tag)
        
        if(args.debug_mode):
            print("[Debug] publishRegexMatchedTags->tag regex input " + str(pattern))

        for tag in tagList:
            if(pattern.match(tag) != None):
                pOpenCommandList = ["git", "ls-remote", "origin", "refs/tags/" + tag]
                status = utils().popenTask(pOpenCommandList)
                output = status[0]
                
                self.excludeFilesForTag(args, tag)

                if(not output):
                    pOpenCommandList = ["git", "push", "origin", tag]
                    status = utils().popenTask(pOpenCommandList)
                    output = status[0]
                    print("[*] pushing tag " + tag + " to remote Github repo")
                else:
                    print("[-] Tag: " + tag + " is already present in the remote Github repo")
            else:
                print("[*] Tag:" + tag + " does not match the specified regex.")

    def publishAllTags(self, args):
        """
        :type: None
        """

        # Push all the tags to Github
        pOpenCommandList = ["git", "tag"]
        status = utils().popenTask(pOpenCommandList)
        output = status[0]
        output = output.split('\n')
        tagList = utils().removeSpaceAndSpecialCharFromList(output)

        for tag in tagList:
            pOpenCommandList = ["git", "ls-remote", "origin", "refs/tags/" + tag]
            status = utils().popenTask(pOpenCommandList)
            output = status[0]

            self.excludeFilesForTag(args, tag)
            
            if(not output):
                pOpenCommandList = ["git", "push", "origin", tag]
                status = utils().popenTask(pOpenCommandList)
                output = status[0]
                print("[*] pushing tag " + tag + " to remote Github repo")
            else:
                print("[-] Tag: " + tag + " is already present in the remote Github repo")

    def publishTheGivenTag(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        pOpenCommandList = ["git", "tag"]
        status = utils().popenTask(pOpenCommandList)
        output = status[0]
        output = output.split('\n')
        tagList = utils().removeSpaceAndSpecialCharFromList(output)

        for tag in tagList:
            if(tag == args.deploy_bitbucket_tag):
                pOpenCommandList = ["git", "ls-remote", "origin", "refs/tags/" + tag]
                status = utils().popenTask(pOpenCommandList)
                output = status[0]
                
                self.excludeFiles(args, tag)

                if(not output):
                    pOpenCommandList = ["git", "push", "origin", tag]
                    status = utils().popenTask(pOpenCommandList)
                    output = status[0]
                    print("[*] pushing tag " + tag + " to remote Github repo")
                else:
                    print("[-] Tag: " + tag + " is already present in the remote Github repo")

            else:
                print("[*] Tag: " + tag + " is not present in the cloned bitbucket repository")

    def publishTags(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        INTERNAL_RELEASE_TAGS="internal"
        PUBLIC_RELEASE_TAGS="public"
        ALL_TAGS="all"
        pattern = re.compile("^(.*?)\.(.*?)\..*$")

        if(args.deploy_bitbucket_tag):
            if(utils().checkSemanticVersioningRegex(args.deploy_bitbucket_tag)):
                if args.debug_mode: print("[Debug] publishTags->checkSemanticVersioningRegex : " + args.deploy_bitbucket_tag)
                self.publishTheGivenTag(args)
            elif(args.deploy_bitbucket_tag == INTERNAL_RELEASE_TAGS):
                if args.debug_mode: print("[Debug] publishTags->publishInternalReleaseTags : " + args.deploy_bitbucket_tag)
                self.publishInternalReleaseTags(args)
            elif(args.deploy_bitbucket_tag == PUBLIC_RELEASE_TAGS):
                if args.debug_mode: print("[Debug] publishTags->publishAllPublicReleaseTags : " + args.deploy_bitbucket_tag)
                self.publishAllPublicReleaseTags(args)
            elif(args.deploy_bitbucket_tag == ALL_TAGS):
                if args.debug_mode: print("[Debug] publishTags->publishAllTags : " + args.deploy_bitbucket_tag)
                self.publishAllTags(args)
            elif(pattern.match(args.deploy_bitbucket_tag)):
                if args.debug_mode: print("[Debug] publishTags->publishRegexMatchedTags : " + args.deploy_bitbucket_tag)
                self.publishRegexMatchedTags(args)
        else:
            if args.debug_mode: print("[Debug] publishTags->publishAllPublicReleaseTags : " + str(args.deploy_bitbucket_tag))
            self.publishAllPublicReleaseTags(args)

    def getRepoPrivateAccessStatus(self, args, githubAccessHook):
        """
        :param args: command line input arguments
        :githubAccessHook: GitHub interface
        :type: Repository private access status
        """

        try:
            repoName = utils().repositoryNameGet(args.deploy_github_url)
            repo = githubAccessHook.get_repo(repoName)
            return repo.private
        except Exception as e:
            raise Exception("[!] Invalid Github repository URL")
    
    def updatePublicRepo(self, args, githubAccessHook):
        """
        :param args: command line input arguments
        :githubAccessHook: GitHub interface
        :type: Repository private access status
        """

        repoName = utils().repositoryNameGet(args.deploy_github_url)

        for repo in githubAccessHook.get_repos():
                # Check repo name
            if(repoName == repo.name):
                repo.edit(
                    private=False
                )
    
    def updatePrivateRepo(self, args, githubAccessHook):
        """
        :param args: command line input arguments
        :param githubAccessHook: GitHub interface
        :type: Repository private access status
        """

        repoName = utils().repositoryNameGet(args.deploy_github_url)

        for repo in githubAccessHook.get_repos():
                # Check repo name
            if(repoName == repo.name):
                repo.edit(
                    private=True
                )

    def gitHubAccessGet(self, args, githubAccess):
        try:
            githubAccessHook = repo().organizationRepoAccessGet(args, githubAccess)
            self.getRepoPrivateAccessStatus(args, githubAccessHook)
            return githubAccessHook
        except Exception as e:
            try:
                args.repo_organization = utils().organizationNameGet(args.deploy_github_url)
                githubAccessHook = repo().organizationRepoAccessGet(args, githubAccess)
                self.getRepoPrivateAccessStatus(args, githubAccessHook)
                return githubAccessHook
            except Exception as e:
                raise Exception("[!] Invalid GitHub repository URL")

    def performDeployOperations(self, args, githubAccess):
        """
        :param args: command line input arguments
        :type: None
        """

        # --deployment
        # --deploy_bitbucket_url
        # --deploy_github_url
        # --deploy_github_user_id
        # --deploy_bitbucket_branch
        # --deploy_bitbucket_tag

        privateRepoStatus = False

        githubAccessHook = self.gitHubAccessGet(args, githubAccess)
        privateRepoStatus = self.getRepoPrivateAccessStatus(args, githubAccessHook)

        if(privateRepoStatus):
            self.updatePublicRepo(args, githubAccessHook)

        print("[*] Starting deployment process at " + utils().currentDataAndTimeGet())

        try:
            self.createWorkSpace()
            self.cloneRepository(args)
            os.chdir(self.workSpaceFolder)
            
            if(args.debug_mode):
                print("[Debug] performDeployOperations->cwd: " +  str(os.getcwd()))

            # Publishing branch
            for branch in args.deploy_bitbucket_branch:
                if(args.debug_mode):
                    print("[Debug] performDeployOperations->branch: " + branch)

                self.checkoutBranch(args, branch)
                if(not args.deploy_mandatory_files_check_disable):
                    utils().isRepoMandatoryFilesArePresent("./", branch)
                self.publishBranch(args, branch)

            # Publishing Tags
            self.publishTags(args)

            if(privateRepoStatus):
                self.updatePrivateRepo(args, githubAccessHook)
        except Exception as e:
            if(privateRepoStatus):
                self.updatePrivateRepo(args, githubAccessHook)
            raise Exception(e)
