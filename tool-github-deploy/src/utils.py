import os
from os import path
import json
import shutil
import ntpath
import subprocess
from subprocess import Popen, PIPE, check_call, STDOUT
import re
import sys
import stat

class utils:
    """
    This class represents utils tool.
    """

    def rmtree(self, inputPath):
        """
        :param inputPath: String
        :type: None
        """

        for root, dirs, files in os.walk(inputPath, topdown=False):
            for name in files:
                filename = os.path.join(root, name)
                os.chmod(filename, stat.S_IWUSR)
                os.remove(filename)
            for name in dirs:
                os.rmdir(os.path.join(root, name))
        os.rmdir(inputPath)    


    def makedirs(self, inputPath):
        """
        :param inputPath: String
        :type: None
        """

        try:
            try:
                self.rmtree(inputPath)
                pass
            except Exception as identifier:
                pass
            os.makedirs(inputPath, exist_ok=True)
        except Exception as identifier:
            pass

    def copytree(self, src, dst, symlinks=False, ignore=None):
        """
        :param src: String
        :param dst: String
        :param symlinks: Boolean
        :param ignore: Boolean
        :type: None
        """

        for item in os.listdir(src):
            s = os.path.join(src, item)
            d = os.path.join(dst, item)
            if os.path.isdir(s):
                shutil.copytree(s, d, symlinks, ignore)
            else:
                shutil.copy2(s, d)

    def boolenStringToBoolean(self, val):
        """
        :param val: String
        :type: Boolean
        """

        if(val == 'true' or val == 'True' or val == True):
            return True
        else:
            return False
    
    def path_leaf(self, path):
        """
        :param path: String
        :type: String
        """

        head, tail = ntpath.split(path)
        return tail or ntpath.basename(head)
    
    def removeSpaceAndSpecialCharFromList(self, oldList):
        """
        :param path: List
        :type: List
        """

        newList = []
        if(oldList):
            for item in oldList:
                if(item):
                    newList.append(item.replace(" ", "").replace(", ", "").replace("'", "").replace("\n", ""))
            return newList
        else:
            return oldList
    
    def popenTask(self, pOpenCommendList):
        """
        :param path: List
        :type: List
        """

        P = Popen(pOpenCommendList, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        output, error = P.communicate()

        return [output, error]

    def checkIsUrlExists(self, url):
        """
        :param path: String
        :type: Boolean
        """

        pOpenCommendList = ["curl", "-s",  "--head", " -w", "%{http_code}", url, "-o", "/dev/null"]
        status = self.popenTask(pOpenCommendList)
        output = status[0]

        if(output.find("200 OK") > -1):
            return True
        else:
            return False
    
    def checkIsGitRepoExists(self, url):
        """
        :param path: String
        :type: Boolean
        """

        pOpenCommendList = ["git", "ls-remote", url]
        P = Popen(pOpenCommendList, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True)
        output, error = P.communicate()

        if(error.find("Repository not found") > -1):
            return False
        else:
            return True

    def checkSemanticVersioningRegex(self, tag):
        """
        :param path: String
        :type: Boolean
        """
        
        SEMVER_REGEX = "^([0-9]+)\.([0-9]+)\.([0-9]+)(?:-([0-9A-Za-z-]+(?:\.[0-9A-Za-z-]+)*))?(?:\+[0-9A-Za-z-]+)?$"

        pattern = re.compile(SEMVER_REGEX)
        if(pattern.match(tag) == None):
            return False
        else:
            return True

    def purge(self, dir, pattern, inclusive=True):
        """
        :param dir: String
        :param pattern: String
        :param inclusive: Boolean
        :type: Boolean
        """

        regexObj = re.compile(pattern)
        git = re.compile("(^.*\.git$)|(^.*.git\\.*$)|(^.*.git\/.*$)")

        for root, dirs, files in os.walk(dir, topdown=False):
            for name in files:
                path = os.path.join(root, name)
                if(git.match(path)):
                    continue
                if bool(regexObj.search(path)) == bool(inclusive):
                    os.remove(path)
            for name in dirs:
                path = os.path.join(root, name)
                if(git.match(path)):
                    continue
                if len(os.listdir(path)) == 0:
                    os.rmdir(path)
    
    def purgeDir(self, dir, pattern, inclusive=True):
        """
        :param dir: String
        :param pattern: String
        :param inclusive: Boolean
        :type: Boolean
        """
        
        regexObj = re.compile(pattern)

        for root, dirs, files in os.walk(dir, topdown=False):
            if bool(regexObj.search(root)) == bool(inclusive):
                self.rmtree(root)        

    
    def isRepoMandatoryFilesArePresent(self, folder, branch):
        """
        :param folder: Folder path
        :param branch: Git branch name
        :type: None
        """

        exceptionString = ""

        fileList = (os.listdir(folder))

        if not ("readme.md" in fileList) and not ("README.md" in fileList):
            exceptionString += "[!] readme.md or README.md file is missing in repository (branch -> "+ branch+")\n"

        if not ("changelog.md" in fileList) and not ("CHANGELOG.md" in fileList):
            exceptionString += "[!] changelog.md or CHANGELOG.md file is missing in repository (branch -> "+ branch+")\n"
        
        if not len([s for s in fileList if "license" in s]) and not len([s for s in fileList if "LICENSE" in s]):
            exceptionString += "[!] license or LICENSE file is missing in repository (branch -> "+ branch+")\n"

        if(exceptionString):
            raise Exception (exceptionString)
    
    def currentDataAndTimeGet(self):
        """
        :type: None
        """
        from datetime import datetime
        return (str(datetime.today().strftime('%Y-%m-%d-%H:%M:%S')))
    
    def githubExceptionWarningMessage(self, message):
        if(message.find("name already exists on this account") > -1):
            print("[-] Skiped Repo creation stage [ Repo name already exists on this account. ]")
        
        if(message.find("Cannot update default branch for an empty repository.") > -1):
            print("[-] Cannot update default branch for an empty repository.  Please init the repository and push first or The branch was not found.")
            pass
        pass

    def githubExceptionErrorMessage(self, message):
        """
        :param message: String
        :type: None
        """

        exceptionString = ""
        
        if(message.find("Bad credentials") > -1):
            exceptionString += ("[!] Invalid GitHub personal access token [Bad credentials]")

        if(message.find("Not Found") > -1):
            exceptionString += ("[!] Unable to find the organization account. Please choose a different organization name.")

        if(message.find("Topics must start with a lowercase letter or number") > -1):
            exceptionString += ("[!] Topics must start with a lowercase letter or number, consist of 35 characters or less, and can include hyphens.")

        if(message.find("[rejected]") > -1):
            urlStr = message.split("[rejected]")[0]
            urlStr = urlStr.replace(" ", "").replace("!", "").replace("\n", "").replace("To", "")
            exceptionString += ("[!] GitHub deploy push rejected ["+ urlStr +"]. [ Do not have permission to push or Create new repository using your GitHub ID ]")

        if(exceptionString):
            raise Exception (exceptionString)
        pass

    def repositoryNameGet(self, urlLink):
        """
        :param urlLink: GitHub Repository URL
        :type: Repository name
        """

        if(urlLink.find(".git") > -1):
            repoUrlSplitList = urlLink.split("/")
            return repoUrlSplitList[len(repoUrlSplitList) - 1].replace(".git")
        else:
            repoUrlSplitList = urlLink.split("/")
            return repoUrlSplitList[len(repoUrlSplitList) - 1]

    def organizationNameGet(self, urlLink):
        """
        :param urlLink: GitHub Repository URL
        :type: organization name
        """

        if(urlLink.find(".git") > -1):
            repoUrlSplitList = urlLink.split("/")
            if(repoUrlSplitList[len(repoUrlSplitList) - 3].find(".com") > -1):
                return repoUrlSplitList[len(repoUrlSplitList) - 2].replace(".git")
        else:
            repoUrlSplitList = urlLink.split("/")
            if(repoUrlSplitList[len(repoUrlSplitList) - 3].find(".com") > -1):
                return repoUrlSplitList[len(repoUrlSplitList) - 2]
        return None

    def errorMsgPrintForhttpStatusCode(self, args, string):
        status = False
        my_path = os.path.abspath(os.path.dirname(__file__))
        httpStatusCodePath = os.path.join(my_path, "httpStatusCode.json")

        jsonDataList = self.readJsonFile(httpStatusCodePath)

        for jsonData in jsonDataList:
            if(string.find(jsonData["code"]) > -1):
                status = True
                print("[!] HTTP response status codes: " + jsonData["code"] + " -> " +
                      jsonData["phrase"] + " (" + jsonData["description"] + ")")
                if(args.debug_mode):
                    print("[!] Original http response message: " + string)
                    print("[!] [Http response code about info]: " +
                          jsonData["spec_href"])
        return status

    def readJsonFile(self, jsonFilePath):
        file = open(jsonFilePath, "r+")
        jsonData = json.loads(file.read())
        file.close()
        return jsonData
    
    def sortSEMVER(self, semverList):
        list1 = [] 
        for x in semverList:
            x = x.replace("'", "").replace("\"", "")
            list1.append(x.split("."))
        list2 = [] 
        for y in list1:
            y = list(map(int, y))
            list2.append(y)
        list3 = sorted(list2)
        FinalList = [] 
        for a in list3:
            a = '.'.join(str(z) for z in a)
            FinalList.append(a)
        return FinalList