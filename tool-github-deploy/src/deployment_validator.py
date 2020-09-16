from jsonschema import validate
from jsonschema import Draft7Validator
from src.utils import utils
from packaging import version

class deploymentValidator:
    def schemaValidator(self, jsonSchema):
        try:
            Draft7Validator.check_schema(jsonSchema)
        except Exception as err:
            raise Exception("[!] Invalid Json schema file formate")

    def validateMetaDataFileWithJsonSchema(self, jsonSchema, metaData):
        try:
            validate(metaData, jsonSchema)
        except Exception as identifier:
            raise Exception(identifier)

    def jsonSchemaOperation(self, args):
        # read json schema file and check syntax issue
        # read meta data file and compare with json schema file
        jsonSchema = ""
        metaData = ""

        try:
            jsonSchema = utils().readJsonFile(args.json_schema_file_path)
        except Exception as identifier:
            print("[!] Invalid Json schema json file format in : " +
                  args.json_schema_file_path)
            pass

        try:
            metaData = utils().readJsonFile(args.meta_data_file_path)
        except Exception as identifier:
            print("[!] Invalid meta data json file format in : " +
                  args.meta_data_file_path)
            pass

        self.schemaValidator(jsonSchema)
        self.validateMetaDataFileWithJsonSchema(jsonSchema, metaData)
        print("[*] No errors found. JSON validates against the schema")
        pass

    def isPresentGitHubReleaseTag(self, args, githubAccess):
        organizationMatch = False
        findRepoStatus = False
        repoList = []
        exceptionString = ""

        try:
            for repo in githubAccess.get_user().get_repos():
                repoList.append(repo.full_name)

                if args.repo_organization:
                    if repo.full_name.find(args.repo_organization) > -1:
                        organizationMatch = True
                    else:
                        organizationMatch = False
                else:
                    organizationMatch = True

                if(args.repo_name == repo.name and organizationMatch):
                    print(
                        "[*] GitHub release tag Repository fullname: " + repo.full_name)

                    findRepoStatus = True
                    # Get tags
                    tagList = []
                    for releaseinfo in repo.get_releases():
                        tagList.append(releaseinfo.title)

                    for releaseInfo in repo.get_releases():
                        # Check tag version
                        if(version.parse(releaseInfo.title) >= version.parse(args.github_release_tag)):
                            currentTagVersion = utils().sortSEMVER(tagList)
                            currentTagVersion = currentTagVersion[len(currentTagVersion) - 1]
                            exceptionString += ("[!] Github release tag you are trying to create '" + args.github_release_tag + "' is lesser than or equal the latest remote Github release : " + str(
                                currentTagVersion) + ". Release should be greater than " + str(currentTagVersion))
                            break
        except Exception as e:
            if(args.debug_mode):
                print("[Debug] Exception: " + str(e))

            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))
            status = utils().errorMsgPrintForhttpStatusCode(args, str(e))
            if(status):
                raise Exception("")

        if(exceptionString):
            raise Exception(exceptionString)
        
        if(not findRepoStatus):
            if(repoList):
                print("[*] We found your repositories in github: " + str(repoList))

            raise Exception(
                "[!] Unable to find github repository name: " + str(args.repo_name))

        print("[*] GitHub release tag version is not available: " +
              str(args.github_release_tag))

    def isPresentGitHubTag(self, args, githubAccess):
        organizationMatch = False
        findRepoStatus = False
        repoList = []
        exceptionString = ""

        try:
            for repo in githubAccess.get_user().get_repos():
                repoList.append(repo.full_name)

                if args.repo_organization:
                    if repo.full_name.find(args.repo_organization) > -1:
                        organizationMatch = True
                    else:
                        organizationMatch = False
                else:
                    organizationMatch = True

                if(args.repo_name == repo.name and organizationMatch):
                    print("[*] GitHub tag Repository fullname: " + repo.full_name)

                    findRepoStatus = True
                    # Get tags
                    tagList = []
                    for taginfo in repo.get_tags():
                        tagList.append(taginfo.name)

                    for taginfo in repo.get_tags():
                        # Check tag version
                        if(version.parse(taginfo.name) > version.parse(args.github_tag)):
                            currentTagVersion = utils().sortSEMVER(tagList)
                            currentTagVersion = currentTagVersion[len(currentTagVersion) - 1]
                            exceptionString += ("[!] Github tag you are trying to create '" + args.github_tag + "' is lesser than or equal the latest remote Github release : " + str(
                                currentTagVersion) + ". Release should be greater than " + str(currentTagVersion))
                            break

        except Exception as e:
            if(args.debug_mode):
                print("[Debug] Exception: " + str(e))

            utils().githubExceptionWarningMessage(str(e))
            utils().githubExceptionErrorMessage(str(e))
            status = utils().errorMsgPrintForhttpStatusCode(args, str(e))
            if(status):
                raise Exception("")

        if(exceptionString):
            raise Exception(exceptionString)

        if(not findRepoStatus):
            if(repoList):
                print("[*] We found your repositories in github: " + str(repoList))

            raise Exception(
                "[!] Unable to find github repository name: " + str(args.repo_name))

        print("[*] GitHub tag version is not available: " + str(args.github_tag))

    def checkRepoIsBelongsToOrganization(self, args, githubAccess):
        repoList = []
        for repo in githubAccess.get_user().get_repos():

            if args.repo_organization:
                if repo.full_name.find(args.repo_organization) > -1:
                    organizationMatch = True
                else:
                    organizationMatch = False
            else:
                organizationMatch = True
            if(args.repo_name == repo.name and organizationMatch):
                repoList.append(repo.full_name)

        if(len(repoList) > 1):
            print(
                "[!] Given repository name is found in multiple places: " + str(repoList))
            raise Exception(
                "[!] Provide valid repository name and Organization name")

    def performDeploymentValidatorOperations(self, args, githubAccess):
        # Json schema
        # Release tag version check
        exceptionString = ""

        if(args.meta_data_file_path and args.json_schema_file_path):
            self.jsonSchemaOperation(args)

        if(githubAccess):
			# validate GitHub release tag version
            self.checkRepoIsBelongsToOrganization(args, githubAccess)

            try:
                if(args.github_release_tag):
                    self.isPresentGitHubReleaseTag(args, githubAccess)
            except Exception as identifier:
                exceptionString += str(identifier) + "\n"
                pass

            # Validate GitHub tag version
            try:
                if(args.github_tag):
                    self.isPresentGitHubTag(args, githubAccess)
                pass
            except Exception as identifier:
                exceptionString += str(identifier) + "\n"
                pass

        if(exceptionString):
            raise Exception(exceptionString)
        pass
