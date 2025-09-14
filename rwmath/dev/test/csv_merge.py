# (c) 2008 Criterion Software Limited and its licensors. All Rights Reserved
# This python script can be used to merge multiple csv files together.
import csv, sys

INITIAL_FIELDS_TO_SKIP = 1

def PrintUsage():
    print """Usage:
    Please supply an output filename followed by a list of files to merge in
    the order you would like them to appear in the output."""

if len(sys.argv) < 2:
    PrintUsage()
else:
    commandLineArgs = sys.argv[1:]
    if '-h' in commandLineArgs or '--help' in commandLineArgs:
        PrintUsage()
    else:
        outputFile = commandLineArgs[0]
        inputFiles = commandLineArgs[1:]
        firstFile = True
        outputData = [['Description']]
        for filename in inputFiles:
            reader = csv.reader(open(filename, "rb"))
            try:
                i = 1
                maxCols = 0
                outputData[0].append(filename)
                for row in reader:
                    # This test will filter the data so only benchmark results are included.  It might be better to make this optional in the future
                    if row[0] != 'BENCHMARK':
                        continue
                    row = row[INITIAL_FIELDS_TO_SKIP:]
                    columnData = row[1:]
                    if firstFile:
                        outputData.append(row)
                    else:
                        if outputData[i][0] != row[0]:
                            raise Exception("Data mismatch between %s and %s." % (inputFiles[0], filename) )
                        outputData[i] = outputData[i] + columnData
                        i = i + 1
                    maxCols = max(maxCols, len(columnData))
                outputData[0] = outputData[0] + (maxCols-1) * ['']

            except csv.Error, e:
                sys.exit('file %s, line %d: %s' % (filename, reader.line_num, e))
            firstFile = False

        writer = csv.writer(open(outputFile, "wb"))
        writer.writerows(outputData)
