from atlassian import Bitbucket
import json
from src.utils import utils

class bitbucket:

    def connectBitBucket(self, args):
        bitbucket_url = "https://bitbucket.microchip.com/"

        try:
            gitAccess = Bitbucket(
                url=bitbucket_url,
                username=args.bitbucket_user_id,
                password=args.bitbucket_personal_access_token)
            
            gitAccess.get_users()
            return gitAccess
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] " + str(e))
            raise Exception("[!] Invlaid bitbucket login information or do not have permission to access this repo")
        
    def getRepo(self, args, gitAccess):
        try:
            gitAccess.get_repo(args.bitbucket_project_name, args.repo_name)
            return True
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] " + str(e))
            return False

    def isTagExists(self, args, tagList):

        for tag in tagList:
            if(tag == args.bitbucket_tag_name):
                return tagList
        return None

    def bitbucketRepoTagListGet(self, args, gitAccess):
        tagList = []

        for tag in gitAccess.get_tags(project=args.bitbucket_project_name, repository=args.repo_name):
            tagList.append(tag["displayId"])
        return tagList

    def create_tag(self, args, gitAccess):
        if(self.getRepo(args, gitAccess)):
            try:
                tagList = self.bitbucketRepoTagListGet(args, gitAccess)

                if(self.isTagExists(args, tagList)):
                    print("[-] Tag already exists: " + str(tagList))
                    return

                gitAccess.set_tag(args.bitbucket_project_name, args.repo_name, args.bitbucket_tag_name, args.bitbucket_commit_hash, args.bitbucket_tag_description)
            except Exception as e:
                utils().errorMsgPrintForhttpStatusCode(args, str(e))
                raise Exception("[!] Either \"invalid Bitbucket repository commit hash\" or \"Repository name or Project name does not exists\" or \"Unauthorized Repository\"")
        else:
            raise Exception("[!] Repository name or Project name does not exists")
        pass


    def performBitbucketOperations(self, args):
        print("[*] Starting Bitbucket tag creation process at " + utils().currentDataAndTimeGet())
        gitAccess = self.connectBitBucket(args)
        self.create_tag(args, gitAccess)