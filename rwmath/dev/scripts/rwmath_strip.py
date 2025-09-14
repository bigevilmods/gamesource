import SimpleCodeStripper

codeStripper = SimpleCodeStripper.Strip(
        additionalSourceFoldersToSkip = ['doc'],
        folderComponentsToExcludeFromCopy=['build', 'test', 'doc'],
        )

# Copy assembly (i.e. .s) files
srcFolders = codeStripper.GenerateRecursiveFolderList(['src'])
srcFoldersToSkip = codeStripper.MatchingFolders(srcFolders, codeStripper.GetDefaultExcludedFolders() )
asmSrcFiles = codeStripper.GenerateFilteredFileList(srcFolders - srcFoldersToSkip , ['.s'])
codeStripper.CopyFiles(asmSrcFiles)
