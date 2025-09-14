import SimpleCodeStripper

codeStripper = SimpleCodeStripper.Strip(
        folderComponentsToExcludeFromCopy=['build', 'examples'],
        )

# Copy doc files
srcFolders = codeStripper.GenerateRecursiveFolderList(['source/doc-lib'])
srcFiles = codeStripper.GenerateFileList(srcFolders)
codeStripper.CopyFiles(srcFiles)
