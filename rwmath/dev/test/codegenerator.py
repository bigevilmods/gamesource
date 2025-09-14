# (c) Electronic Arts. All Rights Reserved.

#------------------------------------------------------------------------------------#
#                                 Code Generator                                     #
#                                                                                    #
#   The following is a library of functions for generating .h and .cpp files for     #
#   Renderware math unit tests.                                                      #
#                                                                                    #
#------------------------------------------------------------------------------------#

import os

#------------------------------------------------------------------------------------#
#                                 Define Constants                                   #
#------------------------------------------------------------------------------------#

# Define types of test class instances
CI_GENERIC = 0                  # Represents rw::math::fpu namespace
CI_VPU = 1                      # Represents rw::math::vpu namespace
CI_VPU_COMPATIBILITY = 2        # Represents rw::math namespace (i.e. for vpu backwards compatibility)
CI_COMMON = 3                   # Represents rw::math namespace (for common tests, e.g. random)
CI_OPTIMAL = 4                  # Represents rw::math::optimal namespace

CI_LIST = [CI_GENERIC, CI_VPU, CI_VPU_COMPATIBILITY, CI_COMMON, CI_OPTIMAL]     # A list of all class instances
CLASS_TYPE_TO_NAMESPACE = { CI_GENERIC:'rw::math::fpu', CI_VPU:'rw::math::vpu', CI_VPU_COMPATIBILITY:'rw::math', CI_COMMON:'rw::math', CI_OPTIMAL:'rw::math::optimal' }
CLASS_TYPE_TO_TEST_ENCLOSING_NAMESPACE = { CI_GENERIC:'generic_fpu', CI_VPU:'host_vpu', CI_VPU_COMPATIBILITY:'host_vpu_compatibility', CI_COMMON:'common_math', CI_OPTIMAL:'platform_optimal' }

# Define misc code strings
BLOCK_DELIM = '//#BLOCK'
CODE_INDENT = '    '
CODE_IFDEF_GEN = '#ifdef GENERIC_TESTING'
CODE_ELSE = '#else'
CODE_ENDIF = '#endif'
CODE_VOID = 'void '
NO_INLINE_PREFIX = 'UNIT_TEST_DECLARE_NO_INLINE_PREFIX '
NO_INLINE_POSTFIX = 'UNIT_TEST_DECLARE_NO_INLINE_POSTFIX'
CODE_INCLUDE = '#include '
CODE_INST_POSTFIX = 'Test'
CODE_DELETE = 'delete'
CODE_SETCLASS = 'SetClassName'
GEN_NAMESPACE = 'rw::math::fpu'

# Define template declarations
TEMP_DECL = 'template<class FloatType>'
TEMP_LIST = '<FloatType>'

# Define template lists
TEMP_FLOAT = '<float>'
TEMP_DOUBLE = '<double>'

# Define file names
MAIN_FILE = 'main.cpp'
TESTINSTANCE_FILE = 'testinstance.cpp'
TEMP_EXT = '.template'
CASES = 'cases'
TEMP_NONE = ''

DEFINES_NONE = ''
DEFINES_OPTIMAL_FPU = '#if defined(RWMATH_FPU_IS_OPTIMAL)'
DEFINES_OPTIMAL_VPU = '#if defined(RWMATH_VPU_IS_OPTIMAL)'

OPTIMAL_NAMESPACE = '\nnamespace rw {namespace math {namespace optimal {}}}\n'

# Define block indices
# Note: Block indices correspond to blocks of codes in .template files. The '//#BLOCK'
# comment in the template files is used to mark the beginning of a block of code.
CODEBLOCK_MAIN_0 = 0
CODEBLOCK_MAIN_1 = 1
CODEBLOCK_MAIN_2 = 2
CODEBLOCK_MAIN_3 = 3
CODEBLOCK_MAIN_4 = 4

CODEBLOCK_HEADER_0 = 0
CODEBLOCK_HEADER_1 = 1

CODEBLOCK_CPP_0 = 0
CODEBLOCK_CPP_1 = 1

CODEBLOCK_TESTINSTANCE_0 = 0
CODEBLOCK_TESTINSTANCE_1 = 1

#------------------------------------------------------------------------------------#
#                                 Class definitions                                  #
#------------------------------------------------------------------------------------#

# --------------------------------- Test class --------------------------------------#
class TestClass:
  def __init__(self, name, header, source, rwhostheader, rwgenericheader, rwcommonheader, rwoptimalheader, tempdecl, templist):
    self.name = name
    self.header = header
    self.source = source
    self.rwhostheader = rwhostheader
    self.rwgenericheader = rwgenericheader
    self.rwcommonheader = rwcommonheader
    self.rwoptimalheader = rwoptimalheader
    self.tempdecl = tempdecl
    self.templist = templist
    self.functions = []

  def write(self, folderName):
    writeHeader(self, folderName, GEN_NAMESPACE)
    writeSource(self, folderName, GEN_NAMESPACE)

# ------------------------------- Test Function -------------------------------------#
class TestFunction:
  def __init__(self, name, code):
    self.name = name
    self.code = code

# ------------------------------ Class Instance -------------------------------------#
class ClassInstance:
  def __init__(self, name, temptypes, classInstanceType, defines):
    self.name = name
    self.temptypes = temptypes
    self.callNames = []
    self.classInstanceType = classInstanceType
    self.defines = defines

  def write(self):
    codeStr = ''
    if (self.defines != DEFINES_NONE):
	  codeStr = codeStr + '\n' + self.defines
    codeStr = codeStr + '\n{'
    codeStr = codeStr + genClassInst(self.name, self.temptypes, self.classInstanceType)
    codeStr = codeStr + genFuncCalls(self.name, self.callNames)
    codeStr = codeStr + genClassDestruct(self.name)
    codeStr = codeStr + '}\n'
    if (self.defines != DEFINES_NONE):
	  codeStr = codeStr + CODE_ENDIF + '\n'	
    return codeStr

  # This function compares two ClassInstances to see if they are "equivalent".  I don't know if
  # this is the proper way to do things in python.
  def isEqual(self, otherInstance):
    return ((self.name == otherInstance.name) and (self.temptypes == otherInstance.temptypes) and (self.classInstanceType == otherInstance.classInstanceType))

#------------------------------------------------------------------------------------#
#                                 Function definitions                               #
#------------------------------------------------------------------------------------#

# -------------------------------- Writes main.cpp ----------------------------------#
def writeCPPs(instanceList, classList, folderName, genSeparateInstanceCPPs):

  if genSeparateInstanceCPPs:
    testInstanceTemplate = os.path.join(CASES, TESTINSTANCE_FILE + TEMP_EXT)
    for instanceType in CI_LIST:
      srcStr = readFileBlock(testInstanceTemplate, CODEBLOCK_TESTINSTANCE_0)
      srcStr = srcStr + genRWInclude(instanceList, classList)
      srcStr = srcStr + OPTIMAL_NAMESPACE
      srcStr = srcStr + genInclude(instanceList, classList, instanceType, genSeparateInstanceCPPs, folderName )

      # Output source file
      testOutfile = os.path.join(folderName, '%s.cpp' % CLASS_TYPE_TO_TEST_ENCLOSING_NAMESPACE[instanceType] )
      testSource = open(testOutfile, 'w')
      testSource.write(srcStr)

      print 'Wrote ' + testOutfile + '...'



  # Get main template
  mainTemplate = os.path.join(CASES, MAIN_FILE + TEMP_EXT)

  # Write next template block
  mainStr = readFileBlock(mainTemplate, CODEBLOCK_MAIN_0);

  # Write rw math headers needed to be included
  mainStr = mainStr + genRWInclude(instanceList, classList)
  # Write forward declare the optimal namespace, in case this is built for the SPU, where it may not include any optimal files but it will have optimal namespace definitions
  mainStr = mainStr + OPTIMAL_NAMESPACE

  # Write out code template's and include directives for all the test classes

  # Write next template block
  mainStr = mainStr + readFileBlock(mainTemplate, CODEBLOCK_MAIN_1) + '\n';

  for instanceType in CI_LIST:
    mainStr = mainStr + genInclude(instanceList, classList, instanceType, not genSeparateInstanceCPPs, folderName )
      
  # Write next template block
  mainStr = mainStr + readFileBlock(mainTemplate, CODEBLOCK_MAIN_2) + '\n';

  # Write class instances and their associated function calls
  for classInst in instanceList:
      if classInst.temptypes == TEMP_FLOAT or classInst.temptypes == TEMP_DOUBLE:
          mainStr = mainStr + classInst.write()

  # Write next template block
  mainStr = mainStr + readFileBlock(mainTemplate, CODEBLOCK_MAIN_3);

  # Write class instances and their associated function calls
  for classInst in instanceList:
      if classInst.temptypes == TEMP_NONE:
          mainStr = mainStr + classInst.write()

  # Write next template block
  mainStr = mainStr + readFileBlock(mainTemplate, CODEBLOCK_MAIN_4);

  # Output source file
  mainOutfile = os.path.join(folderName, MAIN_FILE)
  mainSource = open(mainOutfile, 'w')
  mainSource.write(mainStr)

  print 'Wrote ' + mainOutfile + '...'

# --------------------------- Writes the class header file --------------------------#
def writeHeader(tClass, folderName, namespace):

  # Write the header file
  headerTemplate = os.path.join(CASES, tClass.header + TEMP_EXT)
  headerOutfile = os.path.join(folderName, tClass.header)
  classStr = readFileBlock(headerTemplate, CODEBLOCK_HEADER_0);
  classStr = classStr + genFuncDecl(tClass.name, namespace, tClass.functions);
  classStr = classStr + readFileBlock(headerTemplate, CODEBLOCK_HEADER_1);
  classHeader = open(headerOutfile, 'w')
  classHeader.write(classStr)

  print 'Wrote ' + headerOutfile + '...'

# ---------------------------- Writes the class source file -------------------------#
def writeSource(tClass, folderName, namespace):

  # Write the source file
  sourceTemplate = os.path.join(CASES, tClass.source + TEMP_EXT)
  sourceOutfile = os.path.join(folderName, tClass.source)
  funcStr = readFileBlock(sourceTemplate, CODEBLOCK_CPP_0);
  funcStr = funcStr + genFuncDef(tClass.name, namespace, tClass.functions, tClass.tempdecl, tClass.templist)
  funcStr = funcStr + readFileBlock(sourceTemplate, CODEBLOCK_CPP_1);
  classHeader = open(sourceOutfile, 'w')
  classHeader.write(funcStr)

  print 'Wrote ' + sourceOutfile + '...'

# ------------------------------ Generate function call -----------------------------#
def genFuncCalls(className, callNames):

  codeStr = ''
  for call in callNames:
    codeStr = codeStr + CODE_INDENT + className + CODE_INST_POSTFIX + '->' + call + '();'
    codeStr = codeStr + '\n'

  return codeStr

# ---------------------------- Generate class instantiation -------------------------#
def genClassInst(className, temptypes, classInstanceType):

  enclosingNamespaceStr = CLASS_TYPE_TO_TEST_ENCLOSING_NAMESPACE[classInstanceType] 
  fullClassName = '%s::%s%s' % (enclosingNamespaceStr, className, temptypes)
  codeStr = '\n'
  codeStr = codeStr + CODE_INDENT + fullClassName + '* '
  codeStr = codeStr + className + CODE_INST_POSTFIX + ' = new %s::' % enclosingNamespaceStr
  codeStr = codeStr + className + temptypes + ';\n'
  codeStr = codeStr + CODE_INDENT + className + CODE_INST_POSTFIX + '->'
  codeStr = codeStr + CODE_SETCLASS + '("' + fullClassName + '");'
  codeStr = codeStr + '\n'

  return codeStr

# ------------------------------ Generate class destruction -------------------------#
def genClassDestruct(className):

  codeStr = CODE_INDENT + CODE_DELETE + ' ' + className + CODE_INST_POSTFIX + ';\n'

  return codeStr

# ----------------------------- Generate function definition ------------------------#
def genFuncDecl(className, namespace, functions):

  codeStr = CODE_INDENT + '//===== Function declarations generated by script builder =====\n'
  for func in functions:
    defStr = CODE_INDENT + NO_INLINE_PREFIX + CODE_VOID + func.name + '() ' + NO_INLINE_POSTFIX + ' ;\n'
    codeStr = codeStr + defStr

  return codeStr

# ---------------------------- Generate function definition -----------------------#
def genFuncDef(className, namespace, functions, tempDecl, tempList):

  codeStr = '//===== Functions definitions generated by script builder =====\n\n'
  for func in functions:
    defStr = CODE_IFDEF_GEN + '\n'
    defStr = defStr + tempDecl + '\n'
    defStr = defStr + CODE_VOID + className + tempList + '::' + func.name + '()\n'
    defStr = defStr + CODE_ELSE + '\n' + CODE_VOID + className + '::' + func.name + '()\n'
    defStr = defStr + CODE_ENDIF
    defStr = defStr + '\n{\n\n'
    defStr = defStr + '        SetTestIdentifier("%s");\n' % func.name
    defStr = defStr + func.code
    defStr = defStr + '\n\n}\n\n'
    codeStr = codeStr + defStr

  return codeStr

# ------------------------------ Generate rw math include  -------------------------#
def genRWInclude(instanceList, classList):

  codeStr = ''
  headerList = []

  # Select which headers to include
  for classInst in instanceList:

    toInclude = getRWHeaderList(classInst.name, classList, classInst.classInstanceType)

    # Loop through header files and only include if not already included
    for headerFile in toInclude:
        if not findStr(toInclude, headerList):
            headerList.append(headerFile)
      
  # Generate code string    
  for headerFile in headerList:
    codeStr = codeStr + CODE_INCLUDE + '\"' + headerFile + '\"\n'

  return codeStr

def genOpenNamespace(enclosingNamespaceStr, namespaceStr):
  """Returns a string that opens up each component name of the namespace specified"""
  codeStr = ''
  namespaceList = [enclosingNamespaceStr]
  for nesting, name in enumerate(namespaceList):
    indent = '    ' * nesting
    codeStr = codeStr + '%snamespace %s\n%s{\n' % (indent, name, indent)
  return codeStr

def genCloseNamespace(namespaceStr):
  """Returns a string that closes up each component name of the namespace specified"""
  codeStr = ''
  numNamespaceComponents = 1
  for nesting in range( numNamespaceComponents-1, -1, -1 ):
    codeStr = codeStr + '%s}\n' % ('    ' * nesting, )
  return codeStr


# ------------------------------ Generate include directives ------------------------#
def genInclude(instanceList, classList, classInstanceType, genSeparateInstanceCPPs, folderName):

  testingDefine = None
  if classInstanceType == CI_GENERIC:
      testingDefine = 'GENERIC_TESTING'
  elif classInstanceType == CI_VPU or classInstanceType == CI_VPU_COMPATIBILITY:
      testingDefine = 'HOST_TESTING'
  elif classInstanceType == CI_OPTIMAL:
      testingDefine = 'OPTIMAL_TESTING'

  codeStr = ''
  headerSourceMap = {}

  # Select which headers to include
  for classInst in instanceList:

    # Only include for host/generic
    if(classInst.classInstanceType == classInstanceType):
      toInclude = getClass(classInst.name, classList)
      headerSourceMap[toInclude] = 1

  namespaceStr = CLASS_TYPE_TO_NAMESPACE[classInstanceType]
  enclosingNamespaceStr = CLASS_TYPE_TO_TEST_ENCLOSING_NAMESPACE[classInstanceType] 
  openNamespaceStr = genOpenNamespace( enclosingNamespaceStr, namespaceStr )
  closeNamespaceStr = genCloseNamespace( namespaceStr )

  #
  # Generate code string    
  #

  # Open the namespace
  codeStr = codeStr + openNamespaceStr

  if testingDefine:
    codeStr = codeStr + 'using namespace ::%s;\n' % (namespaceStr, )
    codeStr = codeStr + '#define %s\n' % (testingDefine, )

   # Special define for optimal
  if classInstanceType == CI_OPTIMAL:
    codeStr = codeStr + '#if defined(RWMATH_FPU_IS_OPTIMAL)\n  #define GENERIC_TESTING \n#else\n  #define HOST_TESTING\n#endif\n'        
   
  for toInclude, dummy in headerSourceMap.iteritems():
    headerFile = toInclude.header
    sourceFile = toInclude.source
    codeStr = codeStr + CODE_INCLUDE + '\"' + headerFile + '\"\n'
    if genSeparateInstanceCPPs:
        codeStr = codeStr + CODE_INCLUDE + '\"' + sourceFile + '\"\n'
        # Force instantiations of templated classes for float and double types
        if classInstanceType == CI_OPTIMAL:
          codeStr = codeStr + '#if defined(RWMATH_FPU_IS_OPTIMAL)\n'
        if classInstanceType == CI_GENERIC or classInstanceType == CI_OPTIMAL:
          for templateType in ['float', 'double']:
              codeStr = codeStr + 'template class %s<%s>;\n' % (toInclude.name, templateType)
        if classInstanceType == CI_OPTIMAL:
          codeStr = codeStr + '#endif\n'
			  
   # Special define for optimal
  if classInstanceType == CI_OPTIMAL:
    codeStr = codeStr + '#if defined(RWMATH_FPU_IS_OPTIMAL)\n  #undef GENERIC_TESTING \n#else\n  #undef HOST_TESTING\n#endif\n'        
	
  if testingDefine:
    codeStr = codeStr + '#undef %s\n' % (testingDefine, )

  codeStr = codeStr + closeNamespaceStr


  return codeStr

# ---------------------- Returns rw math header file list for given class -----------#
def getRWHeaderList(className, classObjList, classInstanceType):

  # Check if the item is included in the list
  for classObj in classObjList:
    if (classObj.name == className):
      if (classInstanceType == CI_GENERIC):
        return classObj.rwgenericheader
      if (classInstanceType == CI_VPU or classInstanceType == CI_VPU_COMPATIBILITY):
        return classObj.rwhostheader
      if (classInstanceType == CI_COMMON):
        return classObj.rwcommonheader
      if (classInstanceType == CI_OPTIMAL):
	    return classObj.rwoptimalheader


  return ''

# --------------------------- Returns header file and source file for given class -------------------#
def getClass(className, classObjList):

  # Check if the item is included in the list
  for classObj in classObjList:
    if (classObj.name == className):
      return classObj

  return None

# --------------------------------- Finds a string in list --------------------------#
def findStr(findStr, findList):

  for listItem in findList:
    if (listItem == findStr):
      return True
      
  return False

# ------------------------------------ Write file block -----------------------------#
def readFileBlock(sourceFile, blockNum):

  # Open header for reading
  headerDoc = open(sourceFile, 'r')

  # Read block
  blockCount = -1
  nextline = '\n'
  blockStr = ''
  while (nextline != ''):
    nextline = headerDoc.readline()
    if(nextline.strip().startswith(BLOCK_DELIM)):
      blockCount = blockCount + 1
    elif(blockCount == blockNum):
      blockStr = blockStr + nextline
  blockStr = blockStr + '\n'

  return blockStr

