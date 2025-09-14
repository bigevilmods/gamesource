// (c) 1998-2010 Electronic Arts Inc.

#include <eabase/eabase.h>
#include <eastl/iterator.h>
#include <eastl/vector.h>
#include <eastl/set.h>

#include "EA3ExprFile.h"
#ifdef EA_PLATFORM_MICROSOFT
#pragma warning(disable:4786)
#include <windows.h>
#include <winbase.h>
#else
#include <unistd.h>
//#include <dirent.h>
#include <errno.h>
#endif // EA_PLATFORM_WINDOWS

#include <eastl/set.h>
#include <eastl/vector.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif


using namespace eastl;

namespace EA3Expr {

  // global variable :(
  static const char *currentFileName = NULL;

  Result ForEachFile::eval(EA3Base *b) const {
    //		fprintf(stderr, "inside ForEachFile::eval.\n");
    Result r1 = expr1.eval(b);
    if(r1.type != Result::RS_NAME)
      return Result();
    const char *path = r1.data.str;

    // load a list with directories in path
    char cwdPath[MAX_PATH];
    char *pathString = NULL;
    if (strlen(path)==0) { // cwd only
#ifdef EA_PLATFORM_WINDOWS
      DWORD test = GetCurrentDirectory(MAX_PATH, cwdPath);
      if (test == 0 || test > MAX_PATH)
        return Result();
#else
      //if (getcwd(cwdPath, MAX_PATH) == 0)
        return Result();
#endif // EA_PLATFORM_WINDOWS
      pathString = new char[1+strlen(cwdPath)];
      strcpy(pathString, cwdPath);
    } else { // need to parse path for individual dirs
      pathString = new char[1+strlen(path)];
      if (path && path[0]) { // strtok will modify input string
        strcpy(pathString, path);
      }
    }
#ifdef EA_PLATFORM_WINDOWS
    char seps[] = ";";
#else
    char seps[] = ":";
#endif // EA_PLATFORM_WINDOWS
    eastl::vector<XbString, StlAllocator> dirQueue;
    // load the queue with the dirs in the path
    char *token = strtok(pathString, seps);
    while (token) {
      dirQueue.push_back(XbString(token));
      token = strtok(NULL, ";");
    }
    set<XbString> fileSet;
    // load the set with the paths to each file found
#ifdef EA_PLATFORM_WINDOWS
    WIN32_FIND_DATA fileinfo;
    while (!dirQueue.empty()) {
      XbString fileSpec = dirQueue.front() + "\\*.*";
      HANDLE hSearch = FindFirstFile(fileSpec.getString(), &fileinfo);
      if (hSearch == INVALID_HANDLE_VALUE) {
        dirQueue.erase(dirQueue.begin());
        FindClose(hSearch);
      } else {
        do {
          if (!strcmp(fileinfo.cFileName, ".")) continue;
          if (!strcmp(fileinfo.cFileName, "..")) continue;
          XbString fullPath = dirQueue.front() + "\\" + fileinfo.cFileName;
          if (fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            currentFileName = fullPath.getString();
            // evaluate expr3 with currentFileName set to the current directory
            Result r3 = expr3.eval(b);
            if (r3.type == Result::RS_INT || r3.type == Result::RS_UINT || r3.type == Result::RS_FLOAT) {
              const bool recurse = r3.toInt()?true:false;
              if (recurse) {
                dirQueue.push_back(fullPath);
              }
            }
          } else {
            fileSet.insert(fullPath);
          }
        } while (FindNextFile(hSearch, &fileinfo));
        dirQueue.erase(dirQueue.begin());
        FindClose(hSearch);
      }
    }
#else
    //struct dirent *cur_dir_entry=0;
    //struct stat file_attrs;
    //// breadth first traversal
    //while (!dirQueue.empty()) {
    //  DIR *cur_dir_stream=opendir(dirQueue.front().getString());
    //  if (!cur_dir_stream) {
    //    dirQueue.erase(dirQueue.begin());	 
    //    continue;
    //  }
    //  // step through the current directory, file by file
    //  while ((cur_dir_entry = readdir(cur_dir_stream))) {
    //    // WARNING: readdir is NOT thread-safe - if it's important, use:
    //    // int readdir_r (DirectoryPointer, Entry, Result)
    //    if (!strcmp(cur_dir_entry->d_name, ".")) continue;
    //    if (!strcmp(cur_dir_entry->d_name, "..")) continue;
    //    // get the file attributes of the current file
    //    XbString fullPath = dirQueue.front() + "/" + cur_dir_entry->d_name;
    //    lstat(fullPath.getString(), &file_attrs);
    //    // load the appropriate data structures
    //    currentFileName = fullPath.getString();
    //    Result r3 = expr3.eval(b);
    //    if (r3.type == Result::RS_INT || r3.type == Result::RS_UINT || r3.type == Result::RS_FLOAT) {
    //      const bool recurse = r3.toInt();
    //      if (recurse) {
    //        dirQueue.push_back(fullPath);
    //      }
    //      else if (file_attrs.st_mode & S_IFREG) { // regular file
    //        fileSet.insert(fullPath);
    //      }
    //    }
    //    dirQueue.erase(dirQueue.begin());
    //  }
    //}
#endif // EA_PLATFORM_WINDOWS
      // evaluate expr2 with currentFileName set to the file name or each file found
      Result r;
      set<XbString>::const_iterator setIterator;
      for (setIterator = fileSet.begin(); setIterator != fileSet.end(); setIterator++) {
        currentFileName = setIterator->getString();
        r = expr2.eval(b);
      }
      return r;
    } // ForEachFile::eval

    Result CurrentFilename::eval(EA3Base *b) const {
      if (currentFileName)
        return Result(currentFileName);
      else
        return Result("before the beginning\n");
    } // CurrentFilename::eval

    Result FileOpen::eval(EA3Base *b) const {
      Result r1 = expr1.eval(b);
      Result r2 = expr2.eval(b);
      if(r1.type != Result::RS_NAME || r2.type != Result::RS_NAME)
        return Result();
      const char *mode = r2.data.str;
      const char *filename = r1.data.str;
      FILE *fileStruct = fopen(filename, mode);
      if (fileStruct) {
        //			fprintf(stderr, "file %s opened.\n", filename);
        return fileStruct;
      } else {
        //			fprintf(stderr, "file %s not opened.\n", filename);
        return Result();
      }
    }

    Result _FileClose::eval(EA3Base *b) const {
      Result r = expr.eval(b);
      if(r.type != Result::RS_FILE_PTR)
        return Result();
      FILE *fileStruct = (FILE*)r.data.v;
      if (fclose(fileStruct) == 0) {
        //			fprintf(stderr, "file closed.\n");
        return 1;
      }	else {
        //			fprintf(stderr, "file not closed.\n");
        return Result();
      }
    }

    Result FilePutS::eval(EA3Base *b) const {
      Result r1 = expr1.eval(b);
      Result r2 = expr2.eval(b);
      if(r1.type != Result::RS_FILE_PTR || r2.type != Result::RS_NAME)
        return Result();
      FILE *fileStruct = (FILE*)r1.data.v;
      const char *str = r2.data.str;
      return fputs(str, fileStruct);
    }

    Result _FileGetC::eval(EA3Base *b) const {
      Result r = expr.eval(b);
      if(r.type != Result::RS_FILE_PTR)
        return Result();
      FILE *fileStruct = (FILE*)r.data.v;
      char c[] = {0,0};
      c[0] = static_cast<char>(fgetc(fileStruct));
	  if (*c==EOF) return Result();
      return c;
    }

    Result FileGetLine::eval(EA3Base *b) const {
      Result r1 = expr1.eval(b);
      Result r2 = expr2.eval(b);
      if(r1.type != Result::RS_FILE_PTR)
        return Result();
      if (r2.type != Result::RS_INT && r2.type != Result::RS_UINT && r2.type != Result::RS_FLOAT)
        return Result();
      FILE *fileStruct = (FILE*)r1.data.v;
      int maxChars = (int)r2.toInt();
      char *tmp = new char[maxChars+1];
      tmp = fgets(tmp, maxChars, fileStruct);
	  if (!tmp) return Result();
	  Result r = tmp;
	  delete[] tmp;
      return r;
    }

    Result _FileEOF::eval(EA3Base *b) const {
      Result r = expr.eval(b);
      if(r.type != Result::RS_FILE_PTR)
        return Result();
      FILE *fileStruct = (FILE*)r.data.v;
	  return feof(fileStruct)==0?0:1;
    }

    Result _FileMTime::eval(EA3Base *b) const {
      Result r = expr.eval(b);
      if(r.type != Result::RS_FILE_PTR)
        return Result();
      FILE *fileStruct = (FILE*)r.data.v;
#ifdef EA_PLATFORM_WINDOWS
      int fileHandle = (_fileno)(fileStruct);
      struct _stat fileData;
      if (_fstat(fileHandle, &fileData))
        return Result();
#else
      int fileHandle = fileno(fileStruct);
      struct stat fileData;
      if (fstat(fileHandle, &fileData))
        return Result();
#endif
      return (double) fileData.st_mtime;
    }

    Result _FileSize::eval(EA3Base *b) const {
      Result r = expr.eval(b);
      if(r.type != Result::RS_FILE_PTR)
        return Result();
      FILE *fileStruct = (FILE*)r.data.v;
#ifdef EA_PLATFORM_WINDOWS
      int fileHandle = (_fileno)(fileStruct);
      struct _stat fileData;
      if (_fstat(fileHandle, &fileData))
        return Result();
#else
      int fileHandle = fileno(fileStruct);
      struct stat fileData;
      if (fstat(fileHandle, &fileData))
        return Result();
#endif
      return (int)fileData.st_size;
    }

  } // namespace EA3Expr
