from src.utils import utils


class repo:
    """
    This class represents execute release operations.
    """

    def organizationRepoAccessGet(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: GitHub interface
        """

        try:
            if(args.repo_organization):
                return githubAccess.get_organization(args.repo_organization)
            return githubAccess.get_user()
        except Exception as e:
            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))
            pass

    def createRepo(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: Boolean
        """

        print("[*] Creating a repository...")

        githubAccessHook = self.organizationRepoAccessGet(args, githubAccess)

        if(args.repo_organization):
            print("[*] Creating a repository in the selected organization")

        if(not args.repo_desc):
            args.repo_desc = ""

        try:
            githubAccessHook.create_repo(
                name=args.repo_name,
                description=args.repo_desc,
                has_issues=args.repo_has_issues,
                has_projects=args.repo_has_projects,
                private=args.repo_private)

            return True
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] createRepo: " + str(e))

            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))

    def updateRepo(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: Boolean
        """

        print("[*] Updating a repository...")

        githubAccessHook = self.organizationRepoAccessGet(args, githubAccess)

        if(args.repo_organization):
            print("[*] Updating a repository in the selected organization")

        try:
            for repo in githubAccessHook.get_repos():
                # Check repo name
                if(args.repo_name == repo.name):
                    if(args.repo_desc == None):
                        repo.edit(
                            has_issues=args.repo_has_issues,
                            has_projects=args.repo_has_projects,
                            private=args.repo_private,
                        )
                    else:
                        repo.edit(
                            description=args.repo_desc,
                            has_issues=args.repo_has_issues,
                            has_projects=args.repo_has_projects,
                            private=args.repo_private,
                        )

                    if(args.repo_default_branch):
                        self.updateDefaultBranch(args, repo)

            return True
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] createRepo: " + str(e))

            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))

    def updateDefaultBranch(self, args, repo):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: Boolean
        """

        print("[*] Updating a repository default branch...")

        try:
            repo.edit(
                default_branch=args.repo_default_branch
            )

            return True
        except Exception as e:
            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))

    def validateRepoTopics(self, oldList):
        """
        :param path: List
        :type: List
        """

        newList = []
        tempList = []

        if(oldList):
            for item in oldList:
                itemSpaceSplitList = item.split(" ")
                for itemSpace in itemSpaceSplitList:
                    if(itemSpace):
                        tempList.append(itemSpace)
                item = ['-'.join(tempList)][0]
                tempList = []

                # If GitHub topics hsd unsupported character, Replace with fixed character
                # Space " " -> "Segmented LCD" then the script should add "-" i.e "segmented-lcd"
                # , ->  "Segmented,LCD" then the script should add "-" i.e "segmentedlcd"
                # ' -> "Segmented'LCD'" then the script should add "-" i.e "segmentedlcd"
                # \n -> "Segmented\n" then the script should add "-" i.e "segmented"
                # . -> "Segmented.LCD" then the script should add "-" i.e "segmented-lcd"
                # / -> "Segmented/LCD" then the script should add "-" i.e "segmented-lcd"
                item = item.replace(" ", "-").replace(", ", "").replace("'", "").replace(
                    "\n", "").replace(".", "-").replace("/", "-").replace("\t", "-").replace("\r", "").replace("\\n", "").replace("\\t", "")
                if(item):
                    newList.append(item)

            return newList
        else:
            return oldList

    def updateRepoTopics(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: None
        """

        githubAccessHook = self.organizationRepoAccessGet(args, githubAccess)

        if(args.repo_organization):
            print("[*] Updating a repository topics in the selected organization")

        repoList = []
        findRepoStatus = False

        for repo in githubAccessHook.get_repos():

            # Check repo name
            if(args.repo_name == repo.name):
                print("[*] Updating repository topics...")
                findRepoStatus = True
                repoList.append(repo.full_name)
                args.repo_topics = args.repo_topics.replace(
                    "[", "").replace("]", "").replace('"', '')
                topicsList = args.repo_topics.split(',')
                topicsList = self.validateRepoTopics(topicsList)

                # converting lower case
                topicsList = [x.lower() for x in topicsList]

                if(args.debug_mode):
                    print("[ Debug] topicsList: ")
                    print(topicsList)

                try:
                    repo.replace_topics(topicsList)
                except Exception as e:
                    utils().githubExceptionWarningMessage(str(e))
                    utils().githubExceptionErrorMessage(str(e))

        if(repoList):
            print("[*] Updated github repository topics into : " + str(repoList))

        if(findRepoStatus == False):
            raise Exception(
                "[!] Unable to find github repository name: " + args.repo_name)
        return None

    def performRepoOperations(self, args, githubAccess):
        """
        :param args: command line input arguments
        :param githubAccess: GitHub interface
        :type: Boolean
        """

        print("[*] Starting repository process at " +
              utils().currentDataAndTimeGet())

        if(args.debug_mode):
            print("[Debug] performDeployOperations")

        if(args.repo_update):
            self.updateRepo(args, githubAccess)
        else:
            self.createRepo(args, githubAccess)

        if(args.repo_topics):
            self.updateRepoTopics(args, githubAccess)

        return True
