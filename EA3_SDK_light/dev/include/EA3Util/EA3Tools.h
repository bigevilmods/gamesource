// (c) 1998-2010 Electronic Arts Inc.

//! EA3 tools
/*!
 *  \file EA3Tools.h
 *
 *  Contains EA3 tools.
 */

#ifdef _MSC_VER
#pragma warning(disable : 4251 4710) 
#endif

#define EA3UTIL_NO_COPY(CLASS) private: \
	  CLASS( CLASS const &src );\
	  CLASS const &operator=( CLASS const &src );

#ifndef EA3Tools_H
#define EA3Tools_H

#include "EA3Util.h"
#include <stdio.h> 
#include <stdarg.h> 
//#include <varargs.h>
#ifdef EA_PLATFORM_WINDOWS
#ifndef _stricmp
#define _stricmp _stricmp
#endif
#endif

static const char* EA3UTIL_DLL_VERSION="***EA3_UTIL_VERSION*** 12";

inline const char *getEA3UtilDLLVersion() {
  return EA3UTIL_DLL_VERSION;
}

namespace EA3Util {

/** @name EA3 TOOLS
 *  Base classes for designing EA3 tools.
 */
//@{

//! Tool Status
typedef enum {
  TS_OK,              /*!< everything went well */
  TS_ERROR,           /*!< error executing the tool */
  TS_ERROR_READING,   /*!< error reading file */
  TS_COULD_NOT_OPEN,  /*!< could not open file */
} ToolStatus;

#define MSGERR(code) code ## 1
#define MSGWARN(code) code ## 2
#define MSGINFO(code) code ## 3
#define MSGDEBUG(code) code ## 4

//! Verbose level
typedef enum {
  VERBOSE_NONE = 0, /*!< display no message output */
  VERBOSE_ERROR,    /*!< only display error ouput */
  VERBOSE_WARNING,  /*!< display warnings (and errors) */
  VERBOSE_INFO,     /*!< display info (and warnings and errors) */
  VERBOSE_DEBUG     /*!< display debug output (and info, warnings, and errors) */
} VerboseLevel;

extern EA3UTIL_EXPORT VerboseLevel verboseLevel;

//! \brief resets always_ignore, always_post and was_posted sets
EA3UTIL_EXPORT void errorCodeResetAll();
//! \brief adds code to always_ignore set
EA3UTIL_EXPORT void errorCodeIgnore(int code);
//! \brief adds code to always_post set
EA3UTIL_EXPORT void errorCodeAlwaysPost(int code);
//! \brief checks if code should be posted (returns true) and appends it to the was_posted set if it should be.
EA3UTIL_EXPORT bool errorCodeCheck(int code);
//! \brief checks if code was posted since last time we cleared posted
EA3UTIL_EXPORT bool errorCodeWasPosted(int code);
//! \brief clears the posted set
EA3UTIL_EXPORT void errorCodeClearPosted();


//! Class for outputting messages
/*!
 *  
 * The Messages class provides a way for tools to output various information.
 * All ouput is divided into certain types (errors, debug, etc.) and
 * is shown depending on the current verose level.
 * \par Synopsis
 * \#include "EA3Tools.h"
 * \par MESSAGES TYPES:
 * MSG_ERROR<br>
 * MSG_WARNING<br>
 * MSG_INFO<br>
 * MSG_DEBUG<br>
 * see EA3Util::Messages::Type for details
 * \par EXAMPLE:
 * To ouput debug information in a tool (assuming it has a pointer to a
 * Messages class defined by: Messages *errors, and that the verbose level is
 * greater than or equal to VERBOSE_INFO):
 * \code
 *  EA3Scene scene;
 *  scene.readFile(filename);
 *  EA3Block *env = scene.findByType("envelope");
 *  if( !env ) {
 *    errors->append(Messages::MSG_INFO, "There is no enevelope in %s", filename);
 *  }
 * \endcode
 * You can also use error codes (int) instead of the enum.  The last digit in the code
 * will be used to indicate the verbose level.  1=ERROR,2=WARNING,3=INFO,4=DEBUG.
 * You can use the macro MSGERR, MSGWARN, etc. to append the proper digit.
 * <br>Ie: MSGERR(10000) will give 100001.
 */
class EA3UTIL_EXPORT Messages : public EA3Mem {
public:
  //! Message types
  typedef enum {
    MSG_OUTPUT = -1, /*!< Used to force output, regardless of verbose level */
    MSG_NONE = 0,    /*!< not used */
    MSG_ERROR = 1,  /*!< Error message (a vital operation failed) */
    MSG_WARNING,    /*!< Warning message (something failed, but is not vital to the tool's operation) */
    MSG_INFO,       /*!< Info message (for the user's benefit) */
    MSG_DEBUG,      /*!< Debug message (for the programmer's benefit) */
  } Type;

  //! Callback function
  typedef void Callback(Type t, const char *msg, void *userdata);

  //! Default constructor
  Messages() : callback(NULL) , userData(NULL) {
  };
  //! Default deconstructor
  ~Messages() {
  }

  //! Set the callback for outputting messages
  void setCallback(Callback *cb, void *ud);

  void getCallback(Callback *&cb, void *&ud);

  //! Append a message to the Messages class
  /*!
   * Append a formatted message to the Messages class
   * \par Synopsis
   * \#include "EA3Tools.h"
   * \param code (in) is the type of message (see Messages class)
   * \param format (in) is the format of the message (based on printf in stdio.h)
   */
  void append(int code,const char *format, ...);

  void append(Type t,const char *format, ...);

  void vappend(int code,const char *format,va_list args);

  void vappend(Type t,const char *format,va_list args);

  //! Assignment operator
  void operator= (const char *msg);
  void operator= (const Messages &other);

private:
  //! The message callback
  Callback *callback;
  //! User data for messages callback
  void *userData;
  //! A messages buffer
  static char msgBuf[10240];
};

//! Base class used for adding filters and converters.
/*!
 *  
 * The ToolBase class defines the interface for a
 * function that does something with an EA3Scene. 
 * All filters (like merge, removeBone etc) and
 * converters (like bvh2ea3, etc) derive from this
 * base class and override the appropriate virtual
 * functions.  Follow the links from the
 * <a href="index.html">EA3 home page</a>
 * on extending EA3X for examples.
 * \par Synopsis
 * \#include "EA3Tools.h"
 * \par EXAMPLE:
 * Here's sample code for extending a filter tool:
 * \code
#include "EA3Tools.h"

namespace EA3Util {
class MyTool : public ToolBase {
  public:
    MyTool() : ToolBase(FILTER) {
      myFlag = false;
      myFloat = 0.0f;
      addField("-myfloat","<1.0-2.0> a number between 1.0 and 2.0",this,setMyFloat);
      addFlag("-myflag", "turn myflag on",&myFlag);
    }
    static bool setMyFloat(void  *p,const char  *val) {
      MyTool*tool = (MyTool*)p;
      float f =  atof(val);
      if (f <  1.0 || f  > 2.0) return(false);
      tool->myFloat = f;
      return(true);
    }
    virtual ToolBase *instance() {
      return new MyTool;
    }
    virtual ToolStatus execute(EA3Scene *s, const char *filename,Messages &error) {
      printf("my filter called.\n");
      return(TS_OK);
    }
    virtual const char *getName() const {return "mytool";}
    virtual const char *getDescription() const {
      return "My tool just prints a message.";
    }
    bool myFlag;
    float myFloat;
};
#ifdef EA_PLATFORM_WINDOWS
#define MY_TOOL_EXPORT __declspec(dllexport)
#else
#define MY_TOOL_EXPORT
#endif
extern "C" {
  MY_TOOL_EXPORT ToolBase *ToolInit(ToolList *tl) {
    return new MyTool;
  }
}

}

 * \endcode
 */
class EA3UTIL_EXPORT ToolBase : public EA3Mem {
  public:
    typedef enum {TO_EA3, FROM_EA3, FILTER} ToolType;
    class Field;
//! Class for specifying the command line flags for the tool.
/*!
 *  
 * A Flag is constructed from a name for the flag, the description
 * of the flag and a pointer to a boolean value that will be set 
 * to true when the set() method is called.
 * \par Synopsis
 * \#include "EA3Tools.h"
 */
    class Flag : public EA3Mem {
      public:
        Flag(const char *n,const char *ds,void *p) : name(XbString(n).getString()), desc(XbString(ds).getString()), ptr(p) {
        }
        void set() {
          bool *b = (bool *)ptr;
          *b = true;
        }
        ~Flag() { 
          //free(name);
          //free(desc);
        }
        const char *name;
        const char *desc;

      private:
        friend class Field;
        void *ptr;
    };

//! Class for specifying the command line fields for the tool.
/*!
 *  
 * A Field is constructed from a name for the flag, the description
 * of the field, a type description,a void pointer to pass to the callback,
 * and the callback (a pointer to function that will get called
 * when the field needs to change its value).
 * \par FIELD TYPE DESCRIPTIONS:
 * \anchor ToolBaseFlagFieldTypeDescriptions
 * The type description is used for constructing GUI's for the tool.\n
 * It has the form: (type)[,(type)][,(type)]...\n
 * Each (type) has the form:\n
 * [modifier:]datatype\n
 * - current modifiers: \n
 *   multi    indicates that this field can be specified multiple times.\n
 * - datatypes: \n
 *   int [from to]\n
 *   float [from to]\n
 *   string\n
 *   expression\n
 *   filename\n
 *   directory\n
 *   RGBA   colours each component from 0-255\n
 *   name(<ea3 block type>) ex: name(geometry) to specify the name of a geometry node\n
 *   checkbuttons <name[=value]>[;<name[=value]>]...  default value is name.  Resulting string is the concatination of checkbutton values that are checked.\n
 *   radiobuttons <name[=value]>[;<name[=value]>]...  default value is name.\n
 * - Examples:\n
 *   "int" just an integer\n
 *   "int 10 20" an integer in the range from 10 to 20\n
 *   "float 0.0 100.0"  a floating point number in the range 0.0 to 100.0\n
 *   "checkbuttons X Axis=x;Y Axis=y;Z Axis=z,float" a choice of x,y or z followed by a float\n
 *   "multi:name(bone)"  for example: -rb LThigh -rb RThigh \n
 * \sa EA3Util::ToolBase::addField EA3Util::ToolBase::addFlag
 */
    class Field : public Flag {
      public:
        typedef bool SetCB(void *p, const char *val);
        Field(const char *n,const char *ds,const char *t,void *p,SetCB *s) : Flag(n,ds,p),type(t),scb(s) {
        }
        ~Field() { 
        }
        bool set(const char *val) {
          return (*scb)(ptr,val);
        }
        const char *type;
      private:
        SetCB *scb;
    };
    ToolBase(ToolType t) : toolType(t) {
    }
    virtual ~ToolBase() {
      int i;
      for(i=0;i<fields.getSize();i++) {
        delete fields[i];
      }
      for(i=0;i<flags.getSize();i++) {
        delete flags[i];
      }
    }

    //! Add a field to the current tool class
    /*!
     * Add a field to the current tool class.\n
     * This is for tool options that take arguments.  For example,
     * "-rb bonename" is a field in the removeBone tool for specifying
     * the name of a bone to remove.
     * \par Synopsis
     * \#include "EA3Tools.h"
     * \param name (in) is the name of the field
     * \param desc (in) is a brief description of the field
     * \param type (in) is a
     *   \ref ToolBaseFlagFieldTypeDescriptions "field type description"
     * \param ptr (in) is a pointer to the options class that is passed
     *   field setting callback function.
     * \param sc (in) is a callback function for setting the options based
     *   on the arguments given to the field.
     * \par EXAMPLE:
     *  For specifying a specific LOD for the tool to operate on, the following field is defined:
     * \code
    // This is where the field is defined
    addField("-lod",
"<lodlevel>  the LOD level to operate on",
"int",&opts,setLODlevel);
    // ...
    // This is where the callback function is defined
    static bool setLODlevel(void *ptr, const char *val) {
      MyToolOptions *opts = (MyToolOptions*)ptr;
      int lod = 0;
      sscanf(val, "%i" &lod);
      opts->LOD = lod;
      return(true);
    }
     * \endcode
     * \sa addFlag
     */
    void addField(
    	const char *name,const char *desc,const char *type,void *ptr,Field::SetCB *sc) {
      fields.append(new Field(name,desc,type,ptr,sc));
    }

    //! Add a flag to the current tool class
    /*!
     * Add a flag to the current tool class.\n
     * This is for tool options that do not take arguments.  For example, "-nnc" is 
     * a flag in the the merge tool to stop new channels from being created.
     * \par Synopsis
     * \#include "EA3Tools.h"
     * \param name (in) is the name of the flag
     * \param desc (in) is a brief description of the flag
     * \param ptr (in) is a pointer to the variable to be flagged.
     *  Usually this is cast to a boolean value.
     * \par EXAMPLE:
     *  For specifying to remove all bones, the following flag is defined:
     * \code
    addFlag("-rall",
"             remove all bones except those specified to keep",&opts.remove_all);
     * \endcode
     * \sa addField
     */
    void addFlag(const char *name,const char *desc,void *ptr) {
      flags.append(new Flag(name,desc,ptr));
    }

    int flagOrField(const char *arg) {
      int i;
      for(i=0;i<flags.getSize();i++) {
        if (_stricmp(flags[i]->name,arg)==0) return(1);
      }
      for(i=0;i<fields.getSize();i++) {
        if (_stricmp(fields[i]->name,arg)==0) return(2);
      }
      return(0);
    }
    Flag *getFlag( const char *flagName ) {
      for(int i=0;i<flags.getSize();i++) {
        if (_stricmp(flags[i]->name,flagName)==0) return(flags[i]);
      }
      return(NULL);
    }
    Field *getField( const char *fieldName ) {
      for(int i=0;i<fields.getSize();i++) {
        if (_stricmp(fields[i]->name,fieldName)==0) return(fields[i]);
      }
      return(NULL);
    }
    bool setFlag(const char *name) {
      for(int i=0;i<flags.getSize();i++) {
        if (_stricmp(flags[i]->name,name)==0) {
          flags[i]->set();
          return(true);
        }
      }
      return(false);
    }
    bool setField(const char *name, char *val) {
      for(int i=0;i<fields.getSize();i++) {
        if (_stricmp(fields[i]->name,name)==0) {
          return(fields[i]->set(val));
        }
      }
      return(false);
    }

    //! Tool name.
    /*!
     *  
     *  Returns the filter name if this is a filter tool or
     *  the file extension if this is a converter.
     */
    virtual const char *getName() const = 0;
    virtual const char *getDescription() const {
      return "Programmer of this tool did not provide a description.";
    }
    virtual ToolBase* instance() = 0;
    virtual ToolStatus execute(
    	EA3Scene *s,const char *filename, Messages &messages) = 0;
// DATA
    const ToolType toolType;
    XbPArray<Field *> fields;
    XbPArray<Flag *> flags;
    private:
      ToolBase const &operator=( ToolBase const &src );
};

class EA3UTIL_EXPORT ToolWrapper : public EA3Mem {
public:
  ToolWrapper(ToolBase *tb) : tool(tb) {
  }
  ~ToolWrapper() {
    for(int i=0;i<argv.getSize();i++) {
      EA3Free(argv[i]);
    }
    delete tool;
  }
  ToolBase *getTool() { return tool;}
  void addArg(const char *arg,ToolBase::Field *field) {
    fields.append(field);
    argv.append(XbString::_strdup(arg));
  }
  void addFlag(ToolBase::Flag *flag) {
    fields.append(flag);
    argv.append(NULL);
  }
  XbString getCommandString() const;
  XbString getCommandLog() const;
  ToolStatus execute(EA3Scene *s,const char *filename,Messages &messages);

  ToolBase *tool;
  XbPArray<ToolBase::Flag *> fields;
  XbPArray<char *> argv;
};

class LoadedDLLs;
class EA3UTIL_EXPORT ToolList {
  // Only one ToolList will every be created
  private:
    ToolList(const char *searchPath=NULL);
    
  public:
    //! \brief gets a list of all tools.  
    /*!
     * On first invocation loads every DLL in searchPath or in getenv("EA3UTIL_TOOLS") if searchPath is NULL.
     * Use getMasterToolListDelayLoad to have DLLs load when tool is needed.
     * \param searchPath (in) is your search path
     */
    static ToolList &getMasterToolList(const char *searchPath=NULL);
    static void deleteMasterToolList();
    //! \brief gets a list of all tools.  
    /*!
     * Does not load any DLLs.  parseArgs and processCommandLine (which calls parseArgs) will 
     * load the DLLs as needed.
     * \param searchPath (in) is your search path
     */
    static ToolList &getMasterToolListDelayLoad(const char *searchPath=NULL);
    void tryLoadConverter(const char *option,XbPArray<ToolBase *> &found);
    void unloadDLLs();
    void reloadDLLs(const char *searchPath=NULL);
    void getDLLfilenames(XbPArray<const char*> &filenames,const char *newSearchPath=NULL);
    ToolBase *searchDLL(const char *dllname,const char *toolName=NULL,const char *optionName=NULL);
    ToolBase *getNewTool(const char *name);
    ~ToolList();
  XbPArray<ToolBase *> list;
  XbPArray<void*> dllhandles;
  LoadedDLLs *loadedDLLs;
  int toolcount;
  XbString savedSearchPath;
};
class EA3SaveOptions {
public:
  EA3SaveOptions() {
    binary = false;
    compress = false;
    same = false;
  }
  bool binary;
  bool compress;
  bool same;
};

class EA3Save : public ToolBase {
public:
  EA3Save() : ToolBase(FROM_EA3) {
    addFlag("-bin","save in binary format",&opts.binary);
    addFlag("-com","save compressed",&opts.compress);
    addFlag("-same","save in same format as original file",&opts.same);
    addField("-av","<level> ASCII Verbose level.  0 for none.  2 for max.","int",&opts,setAsciiVerbose);
  };
  static bool setAsciiVerbose(void *,const char *val) {
    ea3asciiVerbose = atoi(val);
    return(true);
  }
  virtual ToolBase *instance() {
    return new EA3Save;
  }
  virtual ToolStatus execute(EA3Scene *s, const char *filename,Messages &error);
  
  virtual const char *getName() const {return ".ea3";}
  virtual const char *getDescription() const {
    return "EA3 File format options.";
  }
  EA3SaveOptions opts;
  EA3UTIL_NO_COPY(EA3Save);

};

class EA3Load : public ToolBase {
  public:
    EA3Load() : ToolBase(TO_EA3) {
    };
    virtual ToolBase *instance() {
      return new EA3Load;
    }
    virtual ToolStatus execute(EA3Scene *s, const char *filename,Messages &error);

    virtual const char *getName() const {return ".ea3";}
    virtual const char *getDescription() const {
      return "";
    }
  EA3UTIL_NO_COPY(EA3Load);
};

//!  Output file type for parseArgs().
typedef enum
{
    MANY_TO_ONE,	/*!< Many input files merging into one output file */
    ONE_TO_ONE,		/*!< Multiple input files, multiple output files. */
    OVERWRITE_SOURCE,	/*!< Output files are the same as inputs. */
} DestinationType;

//!  Parse a command line that contains a list of EA3 commands.
/*!
* \param argc (in) # arguments
* \param argv (in) arguments
* \param filters (out) list of filters in command line
* \param fromConverters (out) list of all possible FROM_EA3 converters
* \param toConverters (out) list of all possible TO_EA3 converters
* \param numFiles (out) # pairs of import/export files
* \param srcFname (out) filenames to import from
* \param dstFname (out) filenames to export to
* \param destType (out) destination type
* \param argumentString (out) argument string
* \param msg (out) messages for the user
* \par Synopsis
* \#include "EA3Tools.h"
*
* The caller is responsible for freeing <I>srcFname</I> and <I>dstFname</I>,
* using free().
* If you want DLLs loaded only when needed, call getMasterToolListDelayLoad before calling this.
*/
EA3UTIL_EXPORT EA3_Status parseArgs(
	int argc,
	char *const argv[],
	XbPArray<ToolWrapper *> &filters,
	XbPArray<ToolWrapper *> &fromConverters,
	XbPArray<ToolWrapper *> &toConverters,
	int &numFiles,
	char **&srcFname,
	char **&dstFname,
	DestinationType &destType,
	XbString &argumentString,
  Messages *msg=NULL);

EA3UTIL_EXPORT void processPipeline(
	EA3Scene &scene, const XbPArray<ToolWrapper *> &filters, Messages &errors );

//! Load a file into an EA3 scene. Conversion into EA3 will be done if the
//! input file extension is not ".ea3".
/*!
* \param scene (in) EA3 scene to load file into
* \param filename (in) filename to be loaded
* \param toConverters (in) list of all available TO_EA3 converters
* \param errors (out) error message
* \par Synopsis
      * \#include "EA3Tools.h"
*/
EA3UTIL_EXPORT ToolStatus loadFile(
        EA3Scene &scene,
	const char *filename,
	const XbPArray<ToolWrapper *> &toConverters,
	Messages &errors );

//! Save a file from an EA3 scene. Conversion from EA3 will be done if the
//! output file extension is not ".ea3".
/*!
* \param scene (in) EA3 scene to save
* \param filename (in) output filename
* \param fromConverters (in) list of all available FROM_EA3 converters
* \param errors (out) error message
* \par Synopsis
* \#include "EA3Tools.h"
*/
EA3UTIL_EXPORT ToolStatus saveFile(
        EA3Scene &scene,
	const char *filename,
	const XbPArray<ToolWrapper *> &fromConverters,
	Messages &errors );

//! Process a pipeline of commands specified via a single string
/*!
* \param scene (in) EA3 scene
* \param cpipeline (in) command pipeline
* \param pushScene (in) whether or not to push the scene
* \param errors (out) error message
* This command basically does the same thing as ea3x, except it is a C funtion
* instead of a executable.  It also only allows loading of files, not saving via 
* the =output syntax.
* Example:
* \code
* processCommandString(myscene,"+removeBone -rb #HasBoneType(chain) skeleton.ea3",true,errors);
* \endcode
* What happens:
* - string is parsed
* - the current contents of the scene are pushed on a stack (if pushScene is true).
* - load each file and convert to ea3 (if any are specified)
* - run all the filters
* - Insert the original scene contents back at beginning of scene's members array. (if pushScene was true)

* \par Synopsis
* \#include "EA3Tools.h"
*/
EA3UTIL_EXPORT ToolStatus processCommandString(
  EA3Scene *scene, const char *cpipeline,bool pushScene,Messages &errors );

typedef ToolBase* GetTool(ToolList *toollist);
typedef ToolBase* GetToolv2(ToolList *toollist,Messages *msg);

EA3UTIL_EXPORT bool shouldUseAppDomains();
EA3UTIL_EXPORT void setShouldUseAppDomains(bool val);

//@}
} //namespace


#endif



