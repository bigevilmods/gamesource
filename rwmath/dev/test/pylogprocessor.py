import re
import csv

pylineRe = re.compile(r'PYTHON_OUTPUT> (.*)')
templatedClassRe = re.compile(r'([^<]*)(<.*>)')

def RefactorClassAndType(test):
    """Normally the float/double template param is stored with the class name.  However,
       since we generally want to group by class (and ignore float/double) we move this
       information into the type.
    """
    oldClass = test['class']
    oldType = test['type']
    mo = templatedClassRe.match(oldClass)
    if mo:
        test['class'] = mo.group(1)
        test['type'] = oldType + mo.group(2)
    return test


def LoadLogFile(fileName):
    """Loads a log file and parses out the embedded python data.  The data is evaluated and returned as a list of test entries."""
    file = open(fileName)
    filteredLines = [ mo.group(1) for mo in [pylineRe.match(line) for line in file.xreadlines()] if mo ]
    return map(RefactorClassAndType, eval( "\n".join(filteredLines) ) )

def GroupLists(matchingAttributes, flatTestList):
    """This function takes a flat list of tests and places them in groups.  Tests
       are considered to be in the same group if they have identical values for the specified
       'matchingAttributes'.
    """
    uniqueGroups = {}
    for test in flatTestList:
        key = []
        value = {}
        for attribute in test.keys():
            if attribute in matchingAttributes:
                key.append(test[attribute])
            else:
                value[attribute] = test[attribute]
        key = tuple(key)
        if not uniqueGroups.has_key(key):
            uniqueGroups[key] = []
        uniqueGroups[key].append(value)
    return uniqueGroups

def FindDictionaryWithType(dictionaryList, type):
    """Given a list of dictionaries, this function returns the first dictionary that has the type attribute with the value specified.
       For example given a set of tests in a group you can find the test that was run on the fpu.
    """
    for dict in dictionaryList:
        if dict['type'] == type:
            return dict
    return None

def ExtractValuesForTypes( groupData, types ):
    """Given a list of types, return a list that contains the value attributes for tests of that type.
       If any one of the types is missing the function returns 'None' instead of a set of partial results.
    """
    matchData = []
    for type in types:
        d = FindDictionaryWithType(groupData, type)
        if not d:
            return None
        else:
            matchData.append(d['value'])
    return matchData

def FlattenAttribute(attribute, groupedTests):
    """This function extracts an attribute that may be unique to each test in a group and promotes it to a common group attribute.
       This operation can end up losing information (if the values for the attribute are not consistent).  It is intended for
       throwing away descriptive information that might vary slightly between tests due to type information in the description.
    """
    result = {}
    for groupKey, groupDictValue in groupedTests.iteritems():
        newGroupKey = None
        for test in groupDictValue:
            if not newGroupKey:
                attributeValue = test[attribute]
                newGroupKey = groupKey[:] + (attributeValue,)
                result[newGroupKey] = []
            del test[attribute]
            
            result[newGroupKey].append(test)

    return result

def BuildFPUVPUComparisonRow(groupPair):
    """This function is used to create performance comparisons between fpu (float) results and vpu results."""
    groupKey, groupValue = groupPair
    row = []
    benchmarkValues = ExtractValuesForTypes( groupValue, ['generic_fpu<float>', 'host_vpu'] )
    if benchmarkValues:
        return list(groupKey) + benchmarkValues
    else:
        print 'Warning: ignoring test data for %s' % (groupKey[2])
        return None

def BuildTypeComparisonRow(groupPair, type1, type2):
    """This function is used to create performance comparisons between fpu (float) results and vpu results."""
    groupKey, groupValue = groupPair
    row = []
    benchmarkValues = ExtractValuesForTypes( groupValue, [type1, type2] )
    if benchmarkValues:
        return list(groupKey) + benchmarkValues
    else:
        print 'Warning: ignoring test data for %s' % (groupKey[2])
        return None

def RenameAttribute(rows, attribute, oldValue, newValue):
    result = []
    for row in rows:
        if row[attribute] == oldValue:
            newRow = dict(row)
            newRow[attribute] = newValue
            result.append(newRow)
    return result

def WriteCSV(fileName, rowData):
    """Writes data to the csv file specified."""
    file = open(fileName, 'wb')
    writer = csv.writer(file)
    for row in rowData:
        writer.writerow(row)
    file.close()


def FPUvsVPU():
    tests = LoadLogFile('opt_log.txt')
    groupedTests = GroupLists(['id', 'class'], tests)
    groupedTests = FlattenAttribute('function', groupedTests)
    rows = [['Test Class', 'Test Identifier', 'Test Description', 'fpu (float)', 'vpu']] + filter(lambda x:x, map(BuildFPUVPUComparisonRow, groupedTests.iteritems()))
    rows.sort(key=lambda row:row[2])
    rows.sort(key=lambda row:row[0])
    WriteCSV('test.csv', rows)

def FPUvsFPU():
    fpu1 = LoadLogFile(r'f:\mac_share\perftest_with_neon_6.txt')
    fpu2 = LoadLogFile(r'c:\rtemp\log2.txt')
    fpu1 = RenameAttribute(fpu1, 'type', 'generic_fpu<float>', 'iphone_fpu<float>')
    tests = fpu1 + fpu2
    groupedTests = GroupLists(['id', 'class'], tests)
    groupedTests = FlattenAttribute('function', groupedTests)
    def FpuFpuCompare(x):
        return BuildTypeComparisonRow(x, 'iphone_fpu<float>', 'generic_fpu<float>')
    rows = [['Test Class', 'Test Identifier', 'Test Description', 'iphone', 'other']] + filter(lambda x:x, map(FpuFpuCompare, groupedTests.iteritems()))
    rows.sort(key=lambda row:row[2])
    rows.sort(key=lambda row:row[0])
    WriteCSV('test.csv', rows)

FPUvsFPU()
