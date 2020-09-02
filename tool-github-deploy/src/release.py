from src.utils import utils

class release:
    """
    This class represents execute release operations.
    """

    def uploadAssetFiles(self, args, releaseInfo):
        """
        :param args: command line input arguments
        :param releaseInfo: GitHub interface
        :type: Boolean
        """

        assetFileList = args.upload_assets.split(',')
        assetFileList = list(set(assetFileList))

        try:
            releaseId = releaseInfo.get_release(args.tag_version)  
        except Exception as identifier:
            if(args.debug_mode):
                print("[Debug] Exception: " + str(identifier))

            print("[-] GitHub Release version not found: " + args.tag_version)
            return False

        try:
            for assetFile in assetFileList:
                releaseId.upload_asset(
                    assetFile, "", "application/zip"
                )
        except Exception as identifier:
            asset_list = []
            for x in releaseId.get_assets():
                fileName = x.browser_download_url.split('/')
                asset_list.append(fileName[len(fileName) - 1])
            
            if(args.debug_mode):
                print("[Debug] Exception: " + str(identifier))

            if(asset_list):
                print("[-] Already_exists Artifact asset files : " + str(asset_list))
            return False
        return True

    def latestReleaseDescriptionGet(self, filepath):
        """
        :param filepath: Changelog file path
        :type: latest release information
        """

        tagStart = False
        releaseStr = ""

        with open(filepath) as fp: 
            Lines = fp.readlines() 
            for line in Lines: 
                if(not tagStart):
                    if(line.find("<a name") >  -1):
                        releaseStr += line
                        tagStart = True
                        continue
                elif(tagStart):
                    if(line.find("<a name") >  -1):
                        return releaseStr
                    releaseStr += line

        return releaseStr

    def releaseDescriptionGet(self, filepath):
        """
        :param filepath: Changelog file path
        :type: latest release information
        """

        file = open(filepath, "r")
        releaseStr = file.read()
        file.close()
        return releaseStr

    def createRelease(self, args, repo):
        """
        :param args: command line input arguments
        :param releaseInfo: GitHub interface
        :type: Boolean
        """

        print("[*] Create a release...")
        
        if(args.release_note_path):
            fileStr = self.releaseDescriptionGet(args.release_note_path)
        else:
            fileStr = ""
        
        if(args.debug_mode):
            print("[Debug] Change log file string: " + fileStr)

        try:
            repo.create_git_release(args.tag_version, args.tag_title, fileStr)
            if(args.upload_assets):
                self.uploadAssetFiles(args, repo)                
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] Exception: " + str(e))

            tagVersionFound = False
            for releaseInfo in repo.get_releases():
                # Check tag version
                if(releaseInfo.title == args.tag_title):
                    tagVersionFound = True
                    print("[-] Already_exists 'Release tag version' : " + str(releaseInfo.title)) 
                    self.editRelease(args, repo, releaseInfo)
            
            if(not tagVersionFound):
                raise Exception("[!] Could not found Release tag version: " + args.tag_title)


        return True

    def editRelease(self, args, repo, releaseInfo):
        """
        :param args: command line input arguments
        :param repo: GitHub repository interface
        :param releaseInfo: GitHub interface
        :type: Boolean
        """

        print("[*] Edit a release...")
        if(args.release_note_path):
            fileStr = self.releaseDescriptionGet(args.release_note_path)
        else:
            fileStr = ""

        if(args.debug_mode):
            print("[Debug] Change log file string: " + fileStr)

        releaseInfo.update_release(args.tag_title, fileStr)

        if(args.upload_assets):
            return self.uploadAssetFiles(args, repo)
        
        return True

    def deleteRelease(self, releaseInfo):
        """
        :param args: command line input arguments
        :param releaseInfo: GitHub interface
        :type: Boolean
        """
        print("[*] Delete a release...")

        releaseInfo.delete_release()
        return True

    def performReleaseOperations(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: Boolean
        """

        if(args.debug_mode):
            print("[Debug] performDeployOperations")

        findTagStatus = False
        tagVersions = []
        findRepoStatus = False
        repoList = []
        organizationMatch = False

        # --release_operation		
        # --release_note_path		
        # --tag_version			
        # --tag_title			    
        # --upload_assets			
        # --edit_release			
        # --delete_release		
        # 
        # --github_personal_access_token 
        # --repo_name			    
        # --debug_mode

        print("[*] Starting release process at " + utils().currentDataAndTimeGet())

        if(args.release_note_path):
            print("[*] Changelog file path: \"" + args.release_note_path +"\"")

        try:
            for repo in githubAccess.get_user().get_repos():
                repoList.append(repo.full_name)
                # Check repo name

                if args.repo_organization:
                    if repo.full_name.find(args.repo_organization) > -1:
                        organizationMatch = True
                    else:
                        organizationMatch = False
                else:
                    organizationMatch = True

                if(args.repo_name == repo.name and organizationMatch):
                    print("[*] Repository fullname: " + repo.full_name)
                    
                    findRepoStatus = True
                    if(args.delete_release or args.edit_release):
                        for releaseInfo in repo.get_releases():
                            tagVersions.append(releaseInfo.title)
                            # Check tag version
                            if(releaseInfo.title == args.tag_title):
                                findTagStatus = True
                                if(args.delete_release):
                                    # delete a release
                                    return self.deleteRelease(releaseInfo)
                                elif(args.edit_release):
                                    # edit a release
                                    return self.editRelease(args, repo, releaseInfo)
                    else:
                        return self.createRelease(args, repo)
            pass
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] Exception: " + str(e))

            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))  

        if(findRepoStatus ==  False):
            print("[!] Unable to find github repository name: " + args.repo_name)
            
            if(repoList):
                print("[*] We found your repositories in github: " + str(repoList))
            return False

        if(findRepoStatus and findTagStatus == False and (args.delete_release or args.edit_release)):
            print("[!] Unable to find github repository tag title: " + args.tag_title)
            
            if(tagVersions):
                print("[*] We found your tag titles in github repository: " + str(tagVersions))
            return False

        return True
