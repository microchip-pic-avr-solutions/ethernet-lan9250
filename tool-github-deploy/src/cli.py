import argparse
from os import path
import os
import getopt
import sys
import shutil
from src.utils import utils
from src.deploy import deploy
import re

class argsData:
    """
    This class represents get command line interface arguments.
    """

    def initializeArgParse(self):
        """
        :type: command line input arguments
        """
        parser = argparse.ArgumentParser(description='tool-github-deploy arguments:')

        # Common args
        parser.add_argument('-gpat', '--github_personal_access_token', help=('Common: argument type String, This argument is used to provide the GitHub personal access token'))
        parser.add_argument('-rpn', '--repo_name', help=('Common: argument type String, This argument is used to provide the GitHub repository name'))
        parser.add_argument('-dm', '--debug_mode', help=('Common:argument type Boolean, This argument is used to set the Debug mode (default state is False)'))
        
        # Repository Args
        parser.add_argument('-rpo', '--repo_operation', help=('repo: argument type Boolean, This argument is used to set the repository operation (default state is False)'))
        parser.add_argument('-rpd', '--repo_desc', help=('repo: argument type String, This argument is used to provide the GitHub repository description'))
        parser.add_argument('-rpt', '--repo_topics', help=('repo: argument type String, This argument is used to provide the GitHub repository topics (comma separate, e.g. -rpt=["PIC24", "dsPIC33"])'))
        parser.add_argument('-rphi', '--repo_has_issues', help=('repo: argument type Boolean, This argument is used to enable or disable the GitHub repositories "issues" option'))
        parser.add_argument('-rphp', '--repo_has_projects', help=('repo: argument type Boolean, This argument is used to enable or disable the GitHub repositories "Projects" option'))
        parser.add_argument('-rpp', '--repo_private', help=('repo: argument type Boolean, This argument is used to make the repository Private or Public'))
        parser.add_argument('-rpdb', '--repo_default_branch', help=('repo: argument type String, This argument is used to set the GitHub repository default branch'))
        parser.add_argument('-rpu', '--repo_update', help=('repo: argument type Boolean, This argument is used to set the GitHub repository updation (default state is False)'))
        parser.add_argument('-rporg', '--repo_organization', help=('repo: argument type String, This argument is used to select the organization where the repo needs to be created. Users can provide the entire URL or just the organization name as the value of this argument.'))

        # Release Args
        parser.add_argument('-rlo', '--release_operation', help=('release: argument type Boolean, This argument is used to set the release operation (default state is False)'))
        parser.add_argument('-rlnp', '--release_note_path', help=('release: argument type String, This argument is used to provide the local release note file path'))
        parser.add_argument('-rltv', '--tag_version', help=('release: argument type String, This argument is used to provide the release tag version'))
        parser.add_argument('-rltt', '--tag_title', help=('release: argument type String, This argument is used to provide the release tag title'))
        parser.add_argument('-rlua', '--upload_assets', help=('release: argument type String, This argument is used to provide the upload artifact assets files (comma separate, e.g. -rlua=archive.zip, support.doc)'))
        parser.add_argument('-rle', '--edit_release', help=('release: argument type Boolean, This argument is used to edit a release in Github Repository (default state is False)'))
        parser.add_argument('-rld', '--delete_release', help=('release: argument type Boolean, This argument is used to delete a release in Github Repository (default state is False)'))

        # Deployment Args
        parser.add_argument('-deploy', '--deployment', help=('deploy:argument type Boolean, This argument is used to set the deployment operation (default state is False)'))
        parser.add_argument('-dburl', '--deploy_bitbucket_url', help=('deploy: argument type String, This argument is used to provide the bitbucket repository URL'))
        parser.add_argument('-dbrf', '--deploy_bitbucket_repo_folder', help=('deploy: argument type String, This argument is used to provide the bitbucket repository folder path'))
        parser.add_argument('-dgurl', '--deploy_github_url', help=('deploy: argument type String, This argument is used to provide the GitHub repository URL'))
        parser.add_argument('-dgid', '--deploy_github_user_id', help=('deploy: argument type String, This argument is used to provide the GitHub user id'))
        parser.add_argument('-dbranch', '--deploy_bitbucket_branch', help=('deploy: argument type String, This argument is used to provide the bitbucket repository branch (comma separate, e.g. -dbranch=master, develop)'))
        parser.add_argument('-dtag', '--deploy_bitbucket_tag', help=('deploy: argument type String, This argument is used to provide the bitbucket repository branch'))
        parser.add_argument('-def', '--deploy_excludes_files', help=('deploy: argument type String [Regex supported], This argument is used to exclude specified files from deployment. (comma separate, e.g. -def="build, dist, config/*.fmpp")'))
        parser.add_argument('-dec', '--deploy_excludes_files_commit', help=('deploy: argument type String, This argument is used to provide the GitHub deploy excludes files commit'))
        parser.add_argument('-dmfd', '--deploy_mandatory_files_check_disable', help=('deploy: argument type Boolean, This argument is used to disable the mandatory files check required for deployment (default state is False)'))

        # Create Tag Args
        parser.add_argument('-bto', '--bitbucket_tag_operation', help=('deploy:argument type Boolean, This argument is used to set the bitbucket tag operation (default state is False)'))
        parser.add_argument('-bid', '--bitbucket_user_id', help=('Common: argument type String, This argument is used to provide the bitbucket user id'))
        parser.add_argument('-bpat', '--bitbucket_personal_access_token', help=('Common: argument type String, This argument is used to provide the bitbucket personal access token'))
        parser.add_argument('-bpn', '--bitbucket_project_name', help=('Common: argument type String, This argument is used to provide the bitbucket project name'))
        parser.add_argument('-btv', '--bitbucket_tag_name', help=('Common: argument type String, This argument is used to provide the bitbucket tag name'))
        parser.add_argument('-bch', '--bitbucket_commit_hash', help=('Common: argument type String, This argument is used to provide the bitbucket commit hash'))
        parser.add_argument('-btd', '--bitbucket_tag_description', help=('Common: argument type String, This argument is used to provide the bitbucket tag description'))

		# read arguments from the command line
        return parser.parse_args()

    def validateMandatoryArgs(self, args):
        """
        :param args: command line input arguments
        :type: Exception
        """

        exceptionString = ""

        if(args.repo_operation or args.repo_update):
            if(not args.github_personal_access_token or not args.repo_name):
                exceptionString += ("[!] Invalid number of arguments: [ usage: tool-github-deploy.py -rpo=true -gpat=<github_personal_access_token> -rpn=<github_repo_name> ... ]")

        if(args.release_operation):
            if(not args.github_personal_access_token or not args.repo_name or not args.tag_version or not args.tag_title):
                exceptionString += ("[!] Invalid number of arguments: [ usage: tool-github-deploy.py -rlo=true -gpat=<github_personal_access_token> -rpn=<github-user-name> -rltv=<tag_version> -rltt=<tag_title> ... ]")
        
        if(args.deployment):
            if(not args.github_personal_access_token or not (args.deploy_bitbucket_url or args.deploy_bitbucket_repo_folder) or not args.deploy_github_url or not args.deploy_github_user_id):
                exceptionString += ("[!] Invalid number of arguments: [ usage: tool-github-deploy.py -deploy=true -gpat=<github_personal_access_token> -dgid=<deploy_github_user_id> -dburl=<deploy_bitbucket_url> -dgurl=<deploy_github_url> ... ]")
        
        if(args.bitbucket_tag_operation):
            if(not args.repo_name or not args.bitbucket_user_id or not args.bitbucket_personal_access_token or not args.bitbucket_project_name or not args.bitbucket_tag_name or not args.bitbucket_commit_hash):
                exceptionString += ("[!] Invalid number of arguments: [ usage: tool-github-deploy.py -bto=true -rpn=<github-user-name> -bid=<bitbucket_user_id> bpat=<bitbucket_personal_access_token> -bpn=<bitbucket_project_name> -btv=<bitbucket_tag_name> -bch=<bitbucket_commit_hash> ... ]")

        if(not args.repo_operation and not args.release_operation and not args.deployment and not args.repo_update and not args.bitbucket_tag_operation):
            exceptionString += ("[!] Should select either one GitHub deploy operation [usage: tool-github-deploy.py -deploy=true ... or tool-github-deploy.py -rlo=true ... or tool-github-deploy.py -rpo=true ... or tool-github-deploy.py -bto=true ...]")

        if(exceptionString):
            raise Exception (exceptionString)

    def validateArgsFolderPath(self, args):
        """
        :param args: command line input arguments
        :type: Exception
        """

        exceptionString = ""
        
        if(args.deployment):
            if(args.deploy_bitbucket_repo_folder and not args.deploy_bitbucket_url):
                if not path.exists(args.deploy_bitbucket_repo_folder):
                    exceptionString += ('[!] Invalid deploy_bitbucket_repo_folder path: ' + str(args.deploy_bitbucket_repo_folder))
                else:
                    # Validate deploy bitbucket folder has git init folder
                    cwd = os.getcwd()
                    os.chdir(args.deploy_bitbucket_repo_folder)
                    pOpenCommandList = ["git", "rev-parse", "--git-dir"]
                    status = utils().popenTask(pOpenCommandList)
                    os.chdir(cwd)
                    if(status[0].find('.git') < 0):
                        exceptionString += ('[!] deploy_bitbucket_repo_folder -> .git folder is missing: ' + str(args.deploy_bitbucket_repo_folder))

        if(args.release_operation):
            if(args.release_note_path):
                args.release_note_path = os.path.join(deploy().workSpaceFolder, args.release_note_path)

                if not path.exists(args.release_note_path):
                    exceptionString += ('[!] Invalid release note file path: ' + args.release_note_path)
        
        if(exceptionString):
            raise Exception (exceptionString)

    def validateURLPath(self, args):
        """
        :param args: command line input arguments
        :type: None
        """
        exceptionString = ""

        if(args.deployment):

            if(args.deploy_bitbucket_url and not args.deploy_bitbucket_repo_folder):
                if(args.debug_mode):
                    print("[Debug] Bitbucket URL: " + args.deploy_bitbucket_url)
                    print("[Debug] GitHub URL: " + args.deploy_github_url)

                if(args.deploy_bitbucket_url.find(".git") > -1):
                    if(not utils().checkIsGitRepoExists(args.deploy_bitbucket_url)):
                        exceptionString += ("[!] Invalid deploy_bitbucket_url path: " + args.deploy_bitbucket_url)
                else:
                    exceptionString += ("[!] Invalid deploy_bitbucket_url path: " + args.deploy_bitbucket_url)
        
            if(args.deploy_github_url):
                gitHubUrl = "https://" + args.deploy_github_user_id + ":"+ args.github_personal_access_token +"@" + args.deploy_github_url.replace("https://","")
                args.deploy_github_url = gitHubUrl

        if(exceptionString):
            raise Exception (exceptionString)
    
    def validateGitReleaseTag(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        INTERNAL_RELEASE_TAGS="internal"
        PUBLIC_RELEASE_TAGS="public"
        ALL_TAGS="all"

        exceptionString = ""

        if(args.deployment):
            if(args.deploy_bitbucket_tag):
                pattern = re.compile("^(.*?)\.(.*?)\..*$")
                if(not utils().checkSemanticVersioningRegex(args.deploy_bitbucket_tag) and (not args.deploy_bitbucket_tag == INTERNAL_RELEASE_TAGS)
                and (not args.deploy_bitbucket_tag == PUBLIC_RELEASE_TAGS) and (not args.deploy_bitbucket_tag == ALL_TAGS) and (None == pattern.match(args.deploy_bitbucket_tag))):
                    exceptionString += ('[!] Invalid deploy_bitbucket_tag version format. usage: \"-dtag=<release-tag-number>\" or \"-dtag=internal\" or \"-dtag=public\" or \"-dtag=all\" or \"-dtag=1.*.* (regex)\"')

        if(exceptionString):
            raise Exception (exceptionString)

    def convertStringToBoolean(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        if(args.repo_has_issues):
            args.repo_has_issues = utils().boolenStringToBoolean(args.repo_has_issues)
        else:
            args.repo_has_issues = True

        if(args.repo_has_projects):
            args.repo_has_projects = utils().boolenStringToBoolean(args.repo_has_projects)
        else:
            args.repo_has_projects = True

        if(args.repo_private):
            args.repo_private = utils().boolenStringToBoolean(args.repo_private)
        else:
            args.repo_private = True

        args.debug_mode = utils().boolenStringToBoolean(args.debug_mode)
        args.repo_operation = utils().boolenStringToBoolean(args.repo_operation)
        args.release_operation = utils().boolenStringToBoolean(args.release_operation)
        args.edit_release = utils().boolenStringToBoolean(args.edit_release)
        args.delete_release = utils().boolenStringToBoolean(args.delete_release)
        args.deployment = utils().boolenStringToBoolean(args.deployment)
        args.repo_update = utils().boolenStringToBoolean(args.repo_update)
        args.bitbucket_tag_operation = utils().boolenStringToBoolean(args.bitbucket_tag_operation)
        args.deploy_mandatory_files_check_disable = utils().boolenStringToBoolean(args.deploy_mandatory_files_check_disable)

    def validateGitBranch(self, args):
        """
        :param args: command line input arguments
        :type: None
        """
        
        if(not args.deploy_bitbucket_branch):
            args.deploy_bitbucket_branch = ["master"]
        else:
            args.deploy_bitbucket_branch = args.deploy_bitbucket_branch.split(',')
            args.deploy_bitbucket_branch = utils().removeSpaceAndSpecialCharFromList(args.deploy_bitbucket_branch)
        pass

    def gitHubOrgNameUpdate(self, args):
        """
        :param args: command line input arguments
        :type: None
        """

        if(args.repo_organization):
            if(args.repo_organization.find("/") > -1):
                # GitHub Organization folder 3 index location
                try:
                    args.repo_organization = args.repo_organization.split("/")[3]
                    if(args.debug_mode):
                        print("[Debug] GitHub Organization folder is : " + args.repo_organization)
                except Exception as e:
                    raise Exception("[!] Invalid GitHub organization URL [" + args.repo_organization + "]. Please provide GitHub URL with organization name")

    def argsGet(self):
        """
        :type: command line input arguments
        """
        try:
            args = self.initializeArgParse()
            self.convertStringToBoolean(args)
            self.validateMandatoryArgs(args)
            self.validateArgsFolderPath(args)
            self.validateURLPath(args)
            self.validateGitReleaseTag(args)
            self.validateGitBranch(args)
            self.gitHubOrgNameUpdate(args)

            return args
        except Exception as e:
            raise e
