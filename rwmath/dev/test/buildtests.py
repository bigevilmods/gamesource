#!/usr/bin/python
# (c) Electronic Arts. All Rights Reserved.

#------------------------------------------------------------------------------------#
#                                 Test Builder                                       #
#------------------------------------------------------------------------------------#

# Number of tests to include per executable (tunable so we can fit the tests on PS3 SPU)
programSize = 20000

# Classes to include in tests
defaultAttribClasses = True
# 'None' indicates all classes should be used.  An empty list can be used to indicate no classes should be used.
attribClass = None

# Types of tests to include
# Note: If a type is not included in this list, all tests that are descendents of 
# this type in the xml tree will be excluded
topLevelAttribs = ['class', 'function', 'operator']
attribType = topLevelAttribs + ['correctness']

#includes optimal tests
buildOptimal = True

# Includes generic tests
buildGeneric = True

# Includes host tests
buildHost = True

# Includes host tests in the rw::math namespace
buildVpuCompatibility = True

# Includes common tests
buildCommon = True

# This flag indicates whether or not the tests CPP/H files should be generated.
generateTests = True

# Shows debug information for this script
showDebug = False

# Flag to indicate whether to generate a batch file for running spu elfs.  The default is false
genResponseFileForSpu = False

# Flag to indicate whether or not separate CPP files should be generated for
# each test instance type (i.e. for vpu/fpu/common etc.)  This option is
# required on Xenon/PC to work around a VS compiler bug related to "leaking
# namespaces".
genSeparateInstanceCPPs = False

targetFolder = '..\\build\\rw_math\\dev\\test\\'

#------------------------------------------------------------------------------------#
#                                 Define Constants                                   #
#------------------------------------------------------------------------------------#

# Define tag names
TAG_ROOT = 'TESTCLASS'
TAG_CLASS = 'CLASS'
TAG_TYPE = 'TYPE'
TAG_TEST = 'TEST'
TAG_BUILD_CONSTRAINT = 'BUILD'

# Define attributes
ATTR_NAME = 'NAME'
ATTR_HEADER = 'HEADER'
ATTR_SOURCE = 'SOURCE'
ATTR_RWHOSTHEADER = 'RWHOSTHEADER'
ATTR_RWGENERICHEADER = 'RWGENERICHEADER'
ATTR_RWCOMMONHEADER = 'RWCOMMONHEADER'
ATTR_RWOPTIMALHEADER = 'RWOPTIMALHEADER'
ATTR_GENERIC = 'GENERIC'
ATTR_HOST = 'HOST'
ATTR_COMMON = 'COMMON'
ATTR_OPTIMAL = 'OPTIMAL'

# Define other constants
VALUE_TRUE = 'true'
VALUE_FALSE = 'false'
PROGRAM_DEFAULT_NAME = 'test'
CODE_NODE_INDEX = 1
TESTCLASS_NODE_INDEX = 0
MAX_PROGRAMS = 10000
UNLIMSIZE = -1
NUM_BUILD_SUB_PROCCESSES = 6
TIMESTAMP_FILENAME = "test-timestamp"

#------------------------------------------------------------------------------------#
#                                 Import libraries                                   #
#------------------------------------------------------------------------------------#

import glob
import re
import os
import getopt
import string
import sys
import codegenerator

from xml.dom import minidom

#------------------------------------------------------------------------------------#
#                                 Class definitions                                  #
#------------------------------------------------------------------------------------#

#--------------------------------- Program Instance ---------------------------------#
class ProgramInstance:
  def __init__(self, name):
    self.name = name
    self.classes = []
    self.instances = []

  def build(self):
    createProgFolder(os.path.join(targetFolder, self.name))
    for classObj in self.classes:
      classObj.write(os.path.join(targetFolder, self.name))
    codegenerator.writeCPPs(self.instances, self.classes, os.path.join(targetFolder, self.name), genSeparateInstanceCPPs)

#------------------------------------------------------------------------------------#
#                                 Function definitions                               #
#------------------------------------------------------------------------------------#

#----------------------------------- Main function ----------------------------------#
def main():

    global attribType, buildGeneric, buildHost, buildVpuCompatibility, buildCommon, programSize, generateTests, attribClass, defaultAttribClasses, targetFolder, genResponseFileForSpu, genSeparateInstanceCPPs
      
    commandLineArgs = sys.argv[1:]
    # Parse command line arguments using getopt
    try:
        options, args = getopt.getopt(commandLineArgs, 'ht:m:b', ['help', 'time-trials=', 'max-program-size', 'build-tests', 'no-generic', 'no-common', 'no-host', 'no-optimal', 'no-vpu-compatibility', 'no-generate-tests', 'test-classes-file=', 'output-dir=', 'generate-spu-response-file', 'generate-ppu-batch-file', 'include-performance-tests', 'only-performance-tests', 'generate-separate-instance-cpps'])
    except getopt.GetoptError:
        printUsage()
        sys.exit(-1)

    # Make sure there are not any arguments that are "left over".
    if len(args) > 0:
        printUsage()
        sys.exit()
        
    for option, arg in options:
        print option
        if option in ('-h', '--help'):
            printUsage()
            sys.exit()
        if option == '--include-performance-tests':
            attribType.append('performance')
        if option == '--only-performance-tests':
            attribType = topLevelAttribs + ['performance']
        if option in ('-m', '--max-program-size'):
            programSize = int(arg)
        if option == '--no-generate-tests':
            generateTests = False;
        if option == '--no-generic':
            buildGeneric = False;
        if option == '--no-common':
            buildCommon = False;
        if option == '--no-host':
            buildHost = False;
	    if option == '--no-optimal':
		    buildOptimal = False;
        if option == '--no-vpu-compatibility':
            buildVpuCompatibility = False;
        if option == '--test-classes-file':
            if defaultAttribClasses:
                # Set defaultAttribClasses to false since the classes are being overrided.
                defaultAttribClasses = False
                # Clear the list of classes
                attribClass = [];
            testFile = open(arg)
            # Read all the lines of text in the file, and strip off the new line characters
            classesInFile = map( string.strip, testFile.readlines() )
            # Add the classes to the 'attribClass' list.
            map( attribClass.append, classesInFile )
        if option == '--output-dir':
            # Fix up separators in the path
            targetFolder = arg.replace('\\', os.sep).replace('/', os.sep)
        if option == '--generate-spu-response-file':
            genResponseFileForSpu = True
        if option == '--generate-separate-instance-cpps':
            genSeparateInstanceCPPs = True
        
    if programSize <= 0:
        print 'The max program size must be positive' 

    if generateTests:
        # Create list of tests to include
        print 'Filtering tests...'
        testList = createTestList()
    
    # Verify test data
    if not verifyTestList(testList):
        print 'Please fix the reported errors before generating the tests.'
        sys.exit(-1)

    # Create list of programs to build
    print 'Generating test data...'
    progList = createPrograms(testList)
  
    # Clear old programs
    print 'Clearing old programs...'  
    clearAllPrograms() 

    # Build programs
    print 'Generating C++ files...'
    buildPrograms(progList)
  
    # Generate batch file for running spu elfs
    if genResponseFileForSpu:
        genSPUResponseFile(len(progList))

    # Create time stamp file
    createTimeStampFile()

# ------------------------------ Create a time stamp file ---------------------------#
def createTimeStampFile():
    tsFile = open( os.path.join(targetFolder, TIMESTAMP_FILENAME), 'w' )
    tsFile.write("""# This is a timestamp file.
# The file may be used to determine if the generated unit test code is older
# than any input files used by the script generator.
# Deleting this file should force the build system to regenerate the unit tests.""")
    tsFile.close()


# -------------------------------- Print usage message ------------------------------#
def printUsage():
  print r"""
Usage:
-b, --build-tests
    Build the test programs using 'make'.
-h, --help
    Print this usage message.
--include-performance-tests
    Include performance tests in the test code that is generated.  (By default
    performance tests are not generated.)
--only-performance-tests
    Only run the performance tests (no correctness tests).
--no-generate-tests
    Do not generate any test CPP/H files.
--no-generic
    Do not generate generic (FPU) tests.
--no-host
    Do not generate host (VPU) tests.
--no-vpu-compatibility
    Do not include tests for vpu compatibility tests (i.e. duplicating
    vpu tests in the rw::math namespace).
--no-parallel-build
    Do not build the unit tests in parallel (i.e. using make -j to spawn
    multiple sub-processes during the build)
-m, --max-program-size=
    Specifies the maximum number of tests to place in a single executable.
--test-classes-file=
    Specifies a file that contains a list of all the classes (as specified in
    the xml template files) that should be output.  If this parameter is not
    specified, a default list of classes will be used.  This parameter may
    be used multiple times to combine multiple class files.
-t, --time-trials=
    Specify the number of times the build process should be timed.
    This option implies --build-tests
--build-makeunittest
    Use main.cpp.template for generating unit tests that can be built via
    "core/math/make unittest".
--output-dir=
    output the test programs to the directory specified.
    (default is ..\build\rw_math\dev\test\)
--generate-separate-instance-cpps
    Generate separate CPP files for each instantiation of the test code.
    (i.e. separate CPPs for fpu/vpu/common)
    
"""

# ------------------- Verify that the data in the test list is correct --------------#
def verifyTestList(testList):
    """This function attempts to verify the test data is correct.  However, it
    may not be able to detect all problems."""
    successFlag = True

    # Make sure there are no duplicate tests.
    # For a given test class there should be no duplicate test names.
    classFunctionTupleDictionary = {}
    for testNode in testList:
        testClassName = getParentClassNode(testNode).getAttributeNode(ATTR_NAME).value
        testFuncName = testNode.getAttributeNode(ATTR_NAME).value
        classFunctionTuple = (testClassName, testFuncName)
        if classFunctionTupleDictionary.has_key(classFunctionTuple):
            print 'Error: Duplicate test name found for ' + testClassName + '::' + testFuncName
            successFlag = False
        else:
            # Add the tuple to the dictionary so we can check for duplicates
            classFunctionTupleDictionary[classFunctionTuple] = None
    return successFlag
        
# ---------------------------------- Build programs ---------------------------------#
def buildPrograms(progList):

  for progInst in progList:
    progInst.build()

# ------------------------------- Create program instances --------------------------#
def createPrograms(testList):

  # Declare program instances
  progList = []

  # Create initial program instance
  progList.append(ProgramInstance(genProgName(0)))

  # Create generic tests (rw::math::fpu namespace)
  if buildGeneric:
     progList = createBuild(testList, codegenerator.TEMP_DECL, codegenerator.TEMP_LIST, 
                            codegenerator.TEMP_FLOAT, progList, codegenerator.CI_GENERIC, codegenerator.DEFINES_NONE)
     
  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildGeneric:
     progList = createBuild(testList, codegenerator.TEMP_DECL, codegenerator.TEMP_LIST, 
                            codegenerator.TEMP_DOUBLE, progList, codegenerator.CI_GENERIC, codegenerator.DEFINES_NONE)

  # Create host tests (rw::math namespace)
  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildHost and buildVpuCompatibility:
     progList = createBuild(testList, codegenerator.TEMP_NONE, codegenerator.TEMP_NONE, 
                            codegenerator.TEMP_NONE, progList, codegenerator.CI_VPU_COMPATIBILITY, codegenerator.DEFINES_NONE)

  # Create common tests (rw::math namespace)
  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildCommon:
     progList = createBuild(testList, codegenerator.TEMP_NONE, codegenerator.TEMP_NONE, 
                            codegenerator.TEMP_NONE, progList, codegenerator.CI_COMMON, codegenerator.DEFINES_NONE)

  # Create host tests (rw::math::vpu namespace)
  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildHost:
     progList = createBuild(testList, codegenerator.TEMP_NONE, codegenerator.TEMP_NONE, 
                            codegenerator.TEMP_NONE, progList, codegenerator.CI_VPU, codegenerator.DEFINES_NONE)

  # Create optimal tests (rw::math::optimal namespace)
  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildOptimal:
     progList = createBuild(testList, codegenerator.TEMP_NONE, codegenerator.TEMP_NONE, 
                            codegenerator.TEMP_NONE, progList, codegenerator.CI_OPTIMAL, codegenerator.DEFINES_OPTIMAL_VPU)

  if(fitBuild(progList, testList) == False):
    progList.append(ProgramInstance(genProgName(len(progList))))
  if buildGeneric:
     progList = createBuild(testList, codegenerator.TEMP_DECL, codegenerator.TEMP_LIST, 
                            codegenerator.TEMP_FLOAT, progList, codegenerator.CI_OPTIMAL, codegenerator.DEFINES_OPTIMAL_FPU)
							
							
  return progList

# ----------------------------------- Create build ----------------------------------#
def createBuild(testList, tempdecl, templist, temptypes, progList, classInstanceType, defines):

  # NOTE: following code assumes tests are grouped by test classes

  # Some variables
  curProgInst = progList[len(progList) - 1]
  curClassInst = codegenerator.ClassInstance
  curClassObj = codegenerator.TestClass
  curFuncObj = codegenerator.TestFunction

  # Sequentially build program instances
  for testNode in testList:

    # Create program instance if it is missing or last one is full
    if(programSize != UNLIMSIZE and getProgSize(curProgInst) >= programSize):
      progList.append(ProgramInstance(genProgName(len(progList))))
      curProgInst = progList[len(progList) - 1]

    # Check if class should be included
    inclGeneric = getParentClassNode(testNode).getAttributeNode(ATTR_GENERIC).value
    inclHost = getParentClassNode(testNode).getAttributeNode(ATTR_HOST).value

    attrCommon = getParentClassNode(testNode).getAttributeNode(ATTR_COMMON)	    
	
    if( attrCommon == None ):
        inclCommon = VALUE_FALSE
    else:
        inclCommon = attrCommon.value

    attrOptimal = getParentClassNode(testNode).getAttributeNode(ATTR_OPTIMAL)
	
    if( attrOptimal == None ):
        inclOptimal = VALUE_FALSE
    else:
        inclOptimal = attrOptimal.value
		
    if( ( classInstanceType == codegenerator.CI_GENERIC and inclGeneric == VALUE_TRUE ) or 
        ( (classInstanceType == codegenerator.CI_VPU or classInstanceType == codegenerator.CI_VPU_COMPATIBILITY ) and inclHost == VALUE_TRUE ) or
        ( classInstanceType == codegenerator.CI_COMMON and inclCommon == VALUE_TRUE ) or
        ( classInstanceType == codegenerator.CI_OPTIMAL and inclOptimal == VALUE_TRUE )):
      
      # Class name of current test
      testClassName = getParentClassNode(testNode).getAttributeNode(ATTR_NAME).value

      # Function name of current test
      testFuncName = testNode.getAttributeNode(ATTR_NAME).value

      # Create class instance if it is missing
      curClassInst = getClassInst(testNode, curProgInst.instances, temptypes, classInstanceType, defines)

      # Get the TestClass object for current test
      curClassObj = getClassObj(testNode, curProgInst.classes, tempdecl, templist)

      # Get the TestFunction object for the current test
      curFuncObj = getFuncObj(testNode, curClassObj.functions)

      # Create function call
      curProgInst.instances[len(curProgInst.instances) - 1].callNames.append(testFuncName)

  return progList

# ---------------------------------- Creates test list ------------------------------#
def createTestList():

  # Create root node
  root = minidom.getDOMImplementation().createDocument(None, "ROOT", None)
  
  testFiles = glob.glob( os.path.join( codegenerator.CASES, '*.xml' ) )

  # Append unit tests
  for testFile in testFiles:
    testRoot = minidom.parse(testFile)
    for node in testRoot.childNodes:
        if node.nodeName == TAG_ROOT:
            root.documentElement.appendChild(node)

  # Filter classes
  classList = root.getElementsByTagName(TAG_CLASS)
  if attribClass:
    for classNode in classList:
      if (filterAttrib(classNode, attribClass) == False):
        classNode.parentNode.removeChild(classNode)

  # Filter types
  typeList = root.getElementsByTagName(TAG_TYPE)
  for typeNode in typeList:
    if (filterAttrib(typeNode, attribType) == False):
      typeNode.parentNode.removeChild(typeNode)

  if(showDebug == True):
    printClasses(root)
    printTypes(root)

  testList = root.getElementsByTagName(TAG_TEST)
  return testList

# ---------------------------- Generates a program instance name --------------------#
def genProgName(index):

  progName = PROGRAM_DEFAULT_NAME + '_' + str(index)
  return progName

# ------------------- Returns class instance associated with the given test ---------#
def getClassInst(testNode, instList, temptypes, classInstanceType, defines):

  # Retrieve test class node
  testClassNode = getParentClassNode(testNode)

  # Search for existing class instance
  testClassName = testClassNode.getAttributeNode(ATTR_NAME).value
  for classInst in instList:
    if classInst.isEqual(codegenerator.ClassInstance(testClassName, temptypes, classInstanceType, defines)):
      return classInst

  # Create new test instance and return it
  instList.append(codegenerator.ClassInstance(testClassName, temptypes, classInstanceType, defines))
  newInst = instList[len(instList) - 1]
  return newInst

# ---------------- Returns test function obj associated with the given test ---------#
def getFuncObj(testNode, funcList):

  # Search for existing test function obj
  testFuncName = testNode.getAttributeNode(ATTR_NAME).value
  for funcItem in funcList:
    if(funcItem.name == testFuncName):
      return funcItem

  # Retrieve other test function information necessary to create object
  testFuncCode = testNode.childNodes[CODE_NODE_INDEX].nodeValue

  # Create new test class object and return it
  funcList.append(codegenerator.TestFunction(testFuncName, testFuncCode))
  newFuncObj = funcList[len(funcList) - 1]
  return newFuncObj

# ---------------- Returns test class obj associated with the given test ------------#
def getClassObj(testNode, classList, tempdecl, templist):

  # Retrieve test class node
  testClassNode = getParentClassNode(testNode)

  # Search for existing test class obj
  testClassName = testClassNode.getAttributeNode(ATTR_NAME).value
  for classItem in classList:
    if(classItem.name == testClassName):
      return classItem

  # Retrieve other test class information necessary to create test object
  testClassHeader = testClassNode.getAttributeNode(ATTR_HEADER).value
  testClassSource = testClassNode.getAttributeNode(ATTR_SOURCE).value
  testClassRWHostHeaderList = testClassNode.getAttributeNode(ATTR_RWHOSTHEADER).value.split()
  testClassRWGenericHeaderList = testClassNode.getAttributeNode(ATTR_RWGENERICHEADER).value.split()
  
  optimalHeaders = testClassNode.getAttributeNode(ATTR_RWOPTIMALHEADER)
  if(optimalHeaders == None):
    optimalHeadersValue = ""
  else:
    optimalHeadersValue = optimalHeaders.value
	
  testClassRWOptimalHeaderList = optimalHeadersValue.split()

  commonHeaders = testClassNode.getAttributeNode(ATTR_RWCOMMONHEADER)
  if(commonHeaders == None):
    commonHeadersValue = ""
  else:
    commonHeadersValue = commonHeaders.value

  testClassRWCommonHeaderList = commonHeadersValue.split()


  # Create new test class object and return it
  classList.append(codegenerator.TestClass(testClassName, testClassHeader, testClassSource, testClassRWHostHeaderList, testClassRWGenericHeaderList, testClassRWCommonHeaderList, testClassRWOptimalHeaderList, tempdecl, templist))
  newClassObj = classList[len(classList) - 1]
  return newClassObj

# ----------- Returns total number of tests for the given program instance ----------#
def getProgSize(progInst):

  testCount = 0
  for classInst in progInst.instances:
    testCount = testCount + len(classInst.callNames)

  return testCount

# --------------------- Returns true if node should be included ---------------------#
def filterAttrib(node, attribList):

  for validAttrib in attribList:
    if(node.getAttribute(ATTR_NAME) == validAttrib):
      return True

  return False

# ---- Returns true if current program instance will fit all tests from a build -----#
def fitBuild(progList, testList):

  # Get current program list
  curProgInst = progList[len(progList) - 1]

  # Get num of tests currently in instance
  numTests = 0
  for classInst in curProgInst.instances:
    numTests = numTests + len(classInst.callNames)
  
  # Get num of addition tests allowed for this instance
  numRemaining = programSize - numTests
  
  # Determine if current program instance will fit all the tests of another build
  if(numRemaining >= testList.length):
    return True
  
  return False

# ---------------- Returns the parent class node for any given node -----------------#
def getParentClassNode(childNode):

  while True:
    if (childNode.parentNode == None):
      exit(0)
    elif (childNode.parentNode.tagName == TAG_CLASS):
      return childNode.parentNode
    else:
      return getParentClassNode(childNode.parentNode)

# ------------------------- Creates a folder a program ------------------------------#
def createProgFolder(progFolderName):

  # Create folder
  print "Creating directory '%s' if necessary..." % progFolderName
  try:
      os.makedirs(progFolderName)
  except OSError, (errno, strerror):
      # Errno 17 indicates the folder exists (at least for win32).  This can be safely ignored
      if not errno == 17:
          print 'Caught OS error (%d) %s'% (errno, strerror)
          sys.exit(-1)

def removeDirectoryRecursively(directory):
    for root, dirs, files in os.walk(directory, topdown=False):
        # remove files first
        for file in files:
            os.remove( os.path.join(root, file) )
        # Then remove directories which should now be empty
        for dir in dirs:
            print os.path.join(root, dir)
            os.rmdir( os.path.join(root, dir) )
    os.rmdir(directory)
  
# ----------------------- Clear all old test programs -------------------------------#
# NOTE this will remove the test directories via "rd test0", "rd test1", etc.
# and breaks out of loop when rd gets called on a nonexistent test folder (which displays
# "The system cannot find the file specified." as expected.)
#
def clearAllPrograms():  

    testFolderExp = re.compile('%s_\d*' % PROGRAM_DEFAULT_NAME)
    for root, dirs, files in os.walk(targetFolder):
        # Loop over the directories in the target folder
        for dir in dirs:
            # If the folder has a name of the form '<PROGRAM_DEFAULT_NAME>_<number>' then delete the directory.
            if testFolderExp.match(dir):
                removeDirectoryRecursively( os.path.join(root, dir) )
  
    
def genSPUResponseFile(numELFs):  

  # Write out 'spu-response.cmd'
  fileHandle = open ( os.path.join(targetFolder, 'spu-response.cmd'), 'w')
  fileContent = '--elf-sequence test_#.elf 0 %d' % ((numELFs-1),)
  fileHandle.write( fileContent )
  fileHandle.close()         

# ----------------------------- Print classes ---------------------------------------#
def printClasses(classRoot):

  print '======= Included classes ========'

  classList = classRoot.getElementsByTagName(TAG_CLASS)

  for classNode in classList:
    print classNode.getAttributeNode(ATTR_NAME).value

  print ''

# ----------- Print types -------------
def printTypes(typeRoot):

  print '======= Included types ========'

  typeList = typeRoot.getElementsByTagName(TAG_TYPE)

  for typeNode in typeList:
    print typeNode.getAttributeNode(ATTR_NAME).value

  print '' 
 
#------------------------------------------------------------------------------------#
#                                 Execute Script                                     #
#------------------------------------------------------------------------------------#

if __name__ == '__main__':
    main()
