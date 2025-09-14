// (c) 1998-2010 Electronic Arts Inc.

/*    DO NOT READ THE HEADER FILE FOR HELP.  USE THE HELP DOCS!   */
/*    IF YOU MUST READ THIS,  START WITH THE EXAMPLES OF THE find */ 
/*    COMMAND.  SEARCH FOR "find("  															*/



#ifndef _EA3_Expr_h_
#define _EA3_Expr_h_

#include "EA3.h"
#include "ea3regexp.h"
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "XbLinear.h"
#include <stdlib.h>
#ifdef toupper
#undef toupper
#endif
#define EA3_BIN_HEADER(name,op)  inline const Expression operator op(const Expression &l, const Expression &r) {return(Expression(l,r,Expression::eval_ ## name)); }

namespace EA3Expr  {
  typedef void PrintCB(const char *msg,void *userData);
  //! Set the callback for the Print expression
  EA3_EXPORT void setPrintCallback(PrintCB *pc,void *userData);
  class  Query;

  //! Result is a class that holds a result of an expression or query.
  /*! 
    It can hold ints, floats, strings, and pointers.  Two results can be added, multiplied 
    or divided.  If either are float, the math is done with floats, if both are ints, the math
    is done with ints.  Otherwise 0 is returned.  Operators && and || are not defined.  If you 
    want to && or || with Results, call toInt().  They are not defined because they do not behave 
    like the built in operators, they will always evaluate the lhs and rhs before calling the
    operator (ie they are never lazy) and this can be very undesirable.
  */
  class ResultsMap;
  class EA3_EXPORT Result : public EA3Mem {
  public:
    //! list of types.
    typedef enum {RS_FLOAT,RS_INT,RS_UINT,RS_NAME,RS_TYPE,RS_BASE,RS_VEC3,RS_MATRIX,RS_UNKNOWN,RS_RM_PTR,RS_FILE_PTR,RS_IMAGE} ResultType;
    // Just used to type convert an unsigned char* to a type used for RS_IMAGE.
		class Image {
		public:
			Image(void *idata) : buffer(idata) {}
			void *buffer;
		};
		//! default constructor
    inline Result() : type(RS_UNKNOWN) {
    };
		void clear();
		~Result();
    //! copy constructor
    inline Result(const Result &r) : type(RS_UNKNOWN) {
      *this = r;
    }
		inline Result(Image i) : type(RS_IMAGE) {
      data.v = i.buffer;
    }
    //! constructor from an int
    inline Result(int i) : type(RS_INT) {
      data.i = i;
    }
	inline Result(long i) : type(RS_INT) {
      data.i = i;
    }

#ifdef  _WIN64
    inline Result(intptr_t i) : type(RS_INT) {
      data.i = i;
    }
    inline Result(uintptr_t i) : type(RS_UINT) {
      data.ui = i;
    }
#endif

    inline Result(const XbVec3f &v) : type(RS_VEC3) {
      data.v = EA3HeapAlloc(3);
			memcpy(data.v,&v,sizeof(v));
    }
    inline Result(const XbMatrix &m) : type(RS_MATRIX) {
      data.v = EA3HeapAlloc(16);
			memcpy(data.v,&m,sizeof(m));
    }
		inline Result(unsigned long i) : type(RS_UINT) {
      data.ui = i;
    }
    inline Result(unsigned int i) : type(RS_UINT) {
      data.ui = i;
    }
    //! constructor from a float
    inline Result(float f) : type(RS_FLOAT) {
      data.f = f;
    }
		inline Result(double f) : type(RS_FLOAT) {
      data.f = (float)f;
    }
    //! constructor from a string
    inline Result(const XbString &str) : type(RS_NAME) {
      data.str = str.getString();
    }
    inline Result(const XbIdentifier &str) : type(RS_TYPE) {
      data.str = str.getString();
    }
    //! constructor from a char *
    inline Result(const char *str) : type(RS_NAME) {
      data.str = XbString(str).getString();
    }
    //! constructor from an EA3Base pointer
    inline Result(const EA3Base *b) : type(RS_BASE) {
      data.v = const_cast<void *>(static_cast<const void*>(b));
    }
    Result(ResultsMap *rm);
		inline Result(FILE *f) : type(RS_FILE_PTR) {
			data.v = static_cast<void *>(f);
		}
    //! convert to an int.  Strings convert to value of str[0]
    inline intptr_t toInt() const {
      switch(type) {
        case RS_INT: return(data.i);
        case RS_FLOAT: return((intptr_t)data.f);
				case RS_UINT: return((intptr_t)data.ui);
				case RS_IMAGE:
				case RS_RM_PTR:
				case RS_FILE_PTR:
        case RS_BASE: return((intptr_t)data.v);
        case RS_NAME: 
        case RS_TYPE: return(data.str[0]);
        default: return(0);
      }
    }
    //! convert to an float.  ints are cast, all others are 0
    inline float toFloat() const {
      switch(type) {
        case RS_UINT: return((float)data.ui);
        case RS_INT: return((float)data.i);
        case RS_FLOAT: return(data.f);
        default: return(0);
      }
    }
    //! assignment
    Result &operator =(const Result &other);
    
    //! struct
    /*!
    \code
    union _data {
      float f;
      int i;
      const char *str;
      void *v;
      XbVec3f data.v3();
      XbMatrix data.m44();
    } data;
    \endcode
    */
		union _data {
			float f;
			intptr_t i;
			uintptr_t ui;
			const char *str;
			void *v;
			inline const XbMatrix &m44() const {
				return *(const XbMatrix*)v;
			}
			inline const XbVec3f &v3() const {
				return *(const XbVec3f*)v;
			}
			inline XbMatrix &m44() {
				return *(XbMatrix*)v;
			}
			inline XbVec3f &v3() {
				return *(XbVec3f*)v;
			}
		} data;
		ResultType type;
		
  };
#define RESULT_FLOAT_OP(_OP_) {\
  if (lhs.type == Result::RS_FLOAT || rhs.type == Result::RS_FLOAT) {\
      return(lhs.toFloat() _OP_ rhs.toFloat());\
    }\
    if (lhs.type == Result::RS_UINT && rhs.type==Result::RS_UINT) {\
      return (lhs.data.ui _OP_ rhs.data.ui);\
    }\
    if (lhs.type == Result::RS_INT || rhs.type==Result::RS_INT) {\
      return (lhs.toInt() _OP_ rhs.toInt());\
    }\
  }
#define RESULT_INT_OP(_OP_) {\
  if (lhs.type == Result::RS_FLOAT || rhs.type == Result::RS_FLOAT) {\
      return(lhs.toInt() _OP_ rhs.toInt());\
    }\
    if (lhs.type == Result::RS_UINT && rhs.type==Result::RS_UINT) {\
      return (lhs.data.ui _OP_ rhs.data.ui);\
    }\
    if (lhs.type == Result::RS_INT || rhs.type==Result::RS_INT) {\
      return (lhs.toInt() _OP_ rhs.toInt());\
    }\
  }
  //! addition, ints are promoted to floats if either is a float
  inline Result operator +(const Result &lhs,const Result &rhs) {
    RESULT_FLOAT_OP(+)
    if (rhs.type == Result::RS_NAME || rhs.type == Result::RS_TYPE) {
      if (lhs.type == Result::RS_NAME) return(XbString(lhs.data.str)+XbString(rhs.data.str));
      if (lhs.type == Result::RS_TYPE) return(XbIdentifier(lhs.data.str)+XbIdentifier(rhs.data.str));
    } else if (rhs.type == Result::RS_VEC3 && lhs.type == Result::RS_VEC3) {
      return(lhs.data.v3() + rhs.data.v3());
    }
    return 0;
  }
  /*! multiplication, ints are promoted to floats if either is a float
  vector * matrix
  matrix * vector
  matrix * matrix
  are defined
  */
  inline Result operator *(const Result &lhs,const Result &rhs) {
    if (rhs.type == Result::RS_MATRIX && lhs.type == Result::RS_MATRIX) {
      return(lhs.data.m44() * rhs.data.m44());
		}
    if (rhs.type == Result::RS_VEC3 && lhs.type == Result::RS_MATRIX) {
      XbVec3f v2;
      lhs.data.m44().multMatrixVec(rhs.data.v3(),v2);
      return v2;
		}
    if (rhs.type == Result::RS_MATRIX && lhs.type == Result::RS_VEC3) {
      XbVec3f v2;
      rhs.data.m44().multVecMatrix(lhs.data.v3(),v2);
      return v2;
		}
		if (rhs.type == Result::RS_VEC3 && lhs.type == Result::RS_FLOAT) {
			return rhs.data.v3() * lhs.data.f;
		}
		if (lhs.type == Result::RS_VEC3 && rhs.type == Result::RS_FLOAT) {
			return lhs.data.v3() * rhs.data.f;
		}
		if (rhs.type == Result::RS_MATRIX && lhs.type == Result::RS_FLOAT) {
			return rhs.data.m44() * lhs.data.f;
		}
		if (lhs.type == Result::RS_MATRIX && rhs.type == Result::RS_FLOAT) {
			return lhs.data.m44() * rhs.data.f;
		}
    RESULT_FLOAT_OP(*)
    return 0;
  }
  //! division, ints are promoted to floats if either is a float
  inline Result operator /(const Result &lhs,const Result &rhs) {
    RESULT_FLOAT_OP(/)
    return 0;
  }
  //! equal, ints are promoted to floats if either is a float
  inline Result operator==(const Result &lhs,const Result &rhs) {
    RESULT_FLOAT_OP(==)
    if ((lhs.type == Result::RS_NAME || lhs.type == Result::RS_TYPE) && (rhs.type == Result::RS_NAME || rhs.type == Result::RS_TYPE)) {
      return(lhs.data.str == rhs.data.str);
    }
    if (lhs.type == Result::RS_BASE && rhs.type == Result::RS_BASE) {
      return(lhs.data.v == rhs.data.v);
    }
    if (lhs.type == Result::RS_RM_PTR && rhs.type == Result::RS_RM_PTR) {
      return(lhs.data.v == rhs.data.v);
    }
    if (lhs.type == Result::RS_VEC3 && rhs.type == Result::RS_VEC3) {
      return(lhs.data.v3() == rhs.data.v3());
    } else if (lhs.type == Result::RS_MATRIX && rhs.type == Result::RS_MATRIX) {
      return(lhs.data.m44() == rhs.data.m44());
    }
    return(0);
  }
  inline Result operator &(const Result &lhs,const Result &rhs) {
    RESULT_INT_OP(&)
    return 0;
  }
  inline Result operator |(const Result &lhs,const Result &rhs) {
    RESULT_INT_OP(|)
    return 0;
  }
  inline Result operator %(const Result &lhs,const Result &rhs) {
    RESULT_INT_OP(%)
    return 0;
  }
  //! not equal, ints are promoted to floats if either is a float
  inline Result operator!=(const Result &lhs,const Result &rhs) {
    return !((lhs==rhs).toInt());
  }
  //! subraction, ints are promoted to floats if either is a float
  inline Result operator -(const Result &lhs, const Result &rhs) {
    RESULT_FLOAT_OP(-)
    if (rhs.type == Result::RS_VEC3 && lhs.type == Result::RS_VEC3) {
      return(lhs.data.v3() - rhs.data.v3());
    }
    return 0;
  }
  //! greater than, ints are promoted to floats it either is a float
  inline bool operator>(const Result &lhs, const Result &rhs) {
    RESULT_FLOAT_OP(>)
    return(0);
  }
  //! less than, ints are promoted to floats it either is a float
  inline bool operator<(const Result &lhs, const Result &rhs) {
    RESULT_FLOAT_OP(<)
    return(0);
  }
  //! less than or equal, ints are promoted to floats it either is a float
  inline bool operator<=(const Result &lhs, const Result &rhs) {
    RESULT_FLOAT_OP(<=)
    return(0);
  }
  //! greater than or equal, ints are promoted to floats it either is a float
  inline bool operator>=(const Result &lhs, const Result &rhs) {
    RESULT_FLOAT_OP(>=)
    return(0);
  }
  //! shift right
  inline Result operator>>(const Result &lhs, const Result &rhs) {
    if (lhs.type == Result::RS_FLOAT || rhs.type == Result::RS_FLOAT) {
      return(lhs.toInt() >> rhs.toInt());
    }
    if (lhs.type == Result::RS_INT) {
      return (lhs.data.i >> rhs.toInt());
    }
    if (lhs.type == Result::RS_UINT) {
      return (unsigned int)(lhs.data.ui >> rhs.toInt());
    } 
    return(0);
  }
  //! shift left
  inline Result operator<<(const Result &lhs, const Result &rhs) {
    if (lhs.type == Result::RS_FLOAT || rhs.type == Result::RS_FLOAT) {
      return(lhs.toInt() << rhs.toInt());
    }
    if (lhs.type == Result::RS_INT) {
      return (lhs.data.i << rhs.toInt());
    }
    if (lhs.type == Result::RS_UINT) {
      return (unsigned int)(lhs.data.ui << rhs.toInt());
    } 
    return(0);
  }
  //! returns !r.toInt()
  inline int operator!(const Result &r) {
    return !r.toInt();
  }
  //! negate, only applies to floats or ints
  inline Result operator-(const Result &r) {
    if (r.type == Result::RS_INT) {
      return(-r.data.i);
    } else if (r.type == Result::RS_UINT) {
      return(-(int)r.data.ui);
    } else if (r.type == Result::RS_FLOAT) {
      return(-r.data.f);
    }
    return(r);
  }


  //! An expression class for holding and evaluating expression trees.
  /*!
    An Expression holds either a constant, a query to evaluate (like HasType("coord")
    or one or two Expression pointers to evaluate (for unary or binary operators) 
    When operators like &&, ||, +, -, etc. are used with Expression's
    the result is to create a new Expression that holds pointers to the left hand side
    and right hand side and the type of operator.  Essentially, the class uses C++'s
    overloaded operators to build the expression tree.  Example:
    <br>
    \code
    Expression a(HasType("coord"); // hold the query HasType 
    Expression b(IsField()); // hold the query IsField
    Expression c(a && b && (GetValue(0)==0.0f)); // build an expression tree.
    \endcode
    See also the find command, it recursed through a hierachy and for
    every EA3Base* if the expression evaluates to non-0 then the pointer
    is added to the list:
    \code
    find(scene,list,HasType("coord") && IsField() && ( GetValue(0) > 100.0f ) );
    \endcode
    The Expression class has an eval method that takes a pointer to an EA3 Object 
    and evaluates the expression with it.  The eval method returns a Result.
    A Result can hold an various types like int, float, string, pointer, etc.  
  */
  class EA3_EXPORT Expression : public EA3Mem {
    public:
      typedef Result EvaluatorCB(const Expression *,EA3Base *);
      static Result eval_OP_AND(const Expression *e,EA3Base *b);
      static Result eval_OP_OR(const Expression *e,EA3Base *b);
      static Result eval_OP_NEQUAL(const Expression *e,EA3Base *b);
      static Result eval_OP_EQUAL(const Expression *e,EA3Base *b);
      static Result eval_OP_BW_AND(const Expression *e,EA3Base *b);
      static Result eval_OP_BW_OR(const Expression *e,EA3Base *b);
      static Result eval_OP_MOD(const Expression *e,EA3Base *b);
      static Result eval_OP_SHIFT_RIGHT(const Expression *e,EA3Base *b);
      static Result eval_OP_SHIFT_LEFT(const Expression *e,EA3Base *b);
      static Result eval_OP_GT(const Expression *e,EA3Base *b);
      static Result eval_OP_LT(const Expression *e,EA3Base *b);
      static Result eval_OP_LTE(const Expression *e,EA3Base *b);
      static Result eval_OP_GTE(const Expression *e,EA3Base *b);
      static Result eval_OP_PLUS(const Expression *e,EA3Base *b);
      static Result eval_OP_MINUS(const Expression *e,EA3Base *b);
      static Result eval_OP_MULT(const Expression *e,EA3Base *b);
      static Result eval_OP_JOIN(const Expression *e,EA3Base *b);
      static Result eval_OP_DIV(const Expression *e,EA3Base *b);
      static Result eval_OP_NOT(const Expression *e,EA3Base *b);
      static Result eval_OP_NEG(const Expression *e,EA3Base *b);
      //! default constructor.  Hold a constant value of 1
      Expression() : val(1) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! copy constructor
      Expression(const Expression &other) {
        rhs = lhs = NULL;
        query = NULL;
        *this = other;
      }
      //! construct a constant expression with an int
      Expression(int v) : val(v) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with a char *
      Expression(const char *v) : val(v) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with an XbString
      Expression(const XbString &v) : val(v) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with an XbIdentifier
      Expression(const XbIdentifier &v) : val(v) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with a float
      Expression(float v) : val(v) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with a base ptr.
      Expression(const EA3Base *b) : val(b) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with a Result.
      Expression(Result r) : val(r) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a constant expression with a ResultsMap ptr.
      Expression(ResultsMap *rm) : val(rm) , evaluatorCB(NULL) {
        rhs = lhs = NULL;
        query = NULL;
      }
      //! construct a expression with a Query
      inline Expression(const Query &q); 
      //! construct a binary expression tree.  Two expressions and an operator.
      Expression(const Expression &l, const Expression &r, EvaluatorCB *ecb) {
        lhs = new Expression(l);
        rhs = new Expression(r);
        query = NULL;
        evaluatorCB = ecb;
      }
      //! construct a unary expression tree.  One expression and an operator.
      Expression(const Expression &other,EvaluatorCB *ecb) {
        lhs = new Expression(other);
        rhs = NULL;
        query = NULL;
        evaluatorCB = ecb;
      }
      ~Expression();
      //! evaluate an expression given an EA3Base pointer.
      inline Result eval(EA3Base *b) const;
      Expression operator!() {
        return(Expression(*this,eval_OP_NOT));
      }
      Expression operator-() {
        return(Expression(*this,eval_OP_NEG));
      }
      //! Assignment operator
      Expression& operator=(const Expression &other);
      
      inline Query *getQuery() const {
        return(query);
      }
    private:
      Result val;
      EvaluatorCB *evaluatorCB;
      Expression *lhs;
      Expression *rhs;
      Query *query;
  };
#ifndef DOXYGEN
  EA3_BIN_HEADER(OP_AND,&&);
  EA3_BIN_HEADER(OP_OR,||);
  EA3_BIN_HEADER(OP_EQUAL,==);
  EA3_BIN_HEADER(OP_NEQUAL,!=);
  EA3_BIN_HEADER(OP_GT,>);
  EA3_BIN_HEADER(OP_LT,<);
  EA3_BIN_HEADER(OP_LTE,<=);
  EA3_BIN_HEADER(OP_GTE,>=);
  EA3_BIN_HEADER(OP_MINUS,-);
  EA3_BIN_HEADER(OP_PLUS,+);
  EA3_BIN_HEADER(OP_MULT,*);
  EA3_BIN_HEADER(OP_DIV,/);
  EA3_BIN_HEADER(OP_BW_AND,&);
  EA3_BIN_HEADER(OP_BW_OR,|);
  EA3_BIN_HEADER(OP_MOD,%);
  EA3_BIN_HEADER(OP_SHIFT_LEFT,<<);
  EA3_BIN_HEADER(OP_SHIFT_RIGHT,>>);
#endif
  inline const Expression join(const Expression &l, const Expression &r) { return(Expression(l,r,Expression::eval_OP_JOIN)); }

  //! find elements in the hierarchy that match the expression given and append to a list
  EA3_EXPORT void  find(EA3Base *root, XbPArray<EA3Base *> &outlist,const Expression &e);
  /*!
    find elements in the hierarchy that match the expression given and append to a list
    Same as find but only recurses on a node if the recurseWhen expression evaluates to true for the node.
  */
  EA3_EXPORT void  find(EA3Base *root, XbPArray<EA3Base *> &outlist,const Expression &e, const Expression &recurseWhen);
  /*! 
    find elements in a list that match the expression given and append to a second list
 
    For example, to find all the bones in the scene that match the name LThigh*:
    \code
    EA3Scene scene;
    scene.readFile(filename);
    XbPArray<EA3Base *> mylist;
    find(&scene,mylist,HasType("bone") && StringMatch("LThigh*",GetName()) );
    \endcode
  */
  EA3_EXPORT void  find(XbPArray<EA3Base *> &inList, XbPArray<EA3Base *> &outList,const Expression &e);
  /*! 
      find the first element in the hierarchy that matches the expression
 
  */
  EA3_EXPORT EA3Pointer  findFirst(EA3Base *root,const Expression &e);
  /*! 
      find the first element in the hierarchy that matches the expression, and only recurse when the recurseWhen expression is true
 
  */
  EA3_EXPORT EA3Pointer  findFirst(EA3Base *root,const Expression &e, const Expression &recurseWhen);
  /*! 
      find the first element in the list that matches the expression
 
  */
  EA3_EXPORT EA3Pointer  findFirst(XbPArray<EA3Base *> &inList,const Expression &e);
  /*!
      Base class for all Queries
 
    You can create your own queries by inheriting from Query.
    I suggest copying one of the Queries in EA3Expr.h.  All the
    source code is in the header file.  Make sure you define the 
    copy constructor! ClassName(const ClassName&)
  */
  class EA3_EXPORT Query : public EA3Mem {
    public: 
      Query() {
      };
      virtual ~Query() {
      };
      virtual Query* copy() const = 0;
      virtual Result eval(EA3Base *b) const = 0;
      virtual Expression operator!() {
          return Expression(*this,Expression::eval_OP_NOT);
      }
    private:
      Query(const Query &) {};
      Query &operator=(const Query &) {return *this;};
  };
  inline Result Expression::eval(EA3Base *b) const {
    if (query) return(query->eval(b));
    if (!evaluatorCB) return(val);
    return (*evaluatorCB)(this,b);
  };
//!   Macro for generating a Query that takes no arguments.
#define EXPR_NO_ARGS_CLASS(CLASS,METHOD) \
  class CLASS : public Query {\
    public:\
      CLASS() {\
      }\
      virtual Result eval(EA3Base *b) const {\
        METHOD;\
      }\
      virtual Query* copy() const {return new CLASS();}\
      CLASS(const CLASS &) {};\
      CLASS &operator=(const CLASS &) {return *this;};\
  };
#define EXPR_NO_ARGS_CLASS_EXPORT(EX,CLASS) \
	class EX CLASS : public Query {\
	  public:\
	    CLASS() {\
	    }\
	    virtual Result eval(EA3Base *b) const;\
	    virtual Query* copy() const {return new CLASS();}\
	    CLASS(const CLASS &) {};\
	    CLASS &operator=(const CLASS &) {return *this;};\
	};
// The next macro renames the class to have an _ prefix.
// This is because the class with one expression as an argument
// would not work correctly because of the copy constructor.
// A dummy argument is added to the class constructor to
// get around this and a function is used to wrap the class.

//!   Macro for generating a Query that takes an Expression as an argument.
#define EXPR_ONE_EXPR_CLASS(CLASS,METHOD) \
  class _ ## CLASS : public Query {\
    public:\
      _ ## CLASS(const Query &q,int ) : expr(q) {\
      }\
      _ ## CLASS(const Expression &e,int) : expr(e) {\
      }\
      virtual Result eval(EA3Base *b) const {METHOD}\
      virtual Query* copy() const {return new _ ## CLASS(expr,0);}\
      _ ## CLASS(const _ ## CLASS &) {\
      }\
      _ ## CLASS &operator=(const _ ## CLASS &) {return *this;};\
      const Expression expr;\
  };\
  inline Expression CLASS(const Expression &e) {\
    return _ ## CLASS(e,0);\
  };
#define EXPR_ONE_EXPR_CLASS_EXPORT(EX,CLASS) \
  class EX _ ## CLASS : public Query {\
    public:\
      _ ## CLASS(const Query &q,int ) : expr(q) {\
      }\
      _ ## CLASS(const Expression &e,int) : expr(e) {\
      }\
      virtual Result eval(EA3Base *b) const;\
      virtual Query* copy() const {return new _ ## CLASS(expr,0);}\
      _ ## CLASS(const _ ## CLASS &) {\
      }\
      _ ## CLASS &operator=(const _ ## CLASS &) {return *this;};\
      const Expression expr;\
  };\
  inline Expression CLASS(const Expression &e) {\
    return _ ## CLASS(e,0);\
  };

//!   Macro for generating a Query that takes two Expressions as arguments.
#define EXPR_TWO_EXPR_CLASS(CLASS,METHOD) \
  class CLASS : public Query {\
    public:\
      CLASS(const Expression &e1,const Expression &e2) : expr1(e1), expr2(e2) {\
      }\
      virtual Result eval(EA3Base *b) const {METHOD;}\
      virtual Query* copy() const {return new CLASS(expr1,expr2);}\
      CLASS(const CLASS &) {\
      }\
      CLASS &operator=(const CLASS &) {return *this;};\
      const Expression expr1,expr2;\
  };
#define EXPR_TWO_EXPR_CLASS_EXPORT(EX,CLASS) \
  class EX CLASS : public Query {\
    public:\
      CLASS(const Expression &e1,const Expression &e2) : expr1(e1), expr2(e2) {\
      }\
      virtual Result eval(EA3Base *b) const;\
      virtual Query* copy() const {return new CLASS(expr1,expr2);}\
      CLASS(const CLASS &) {\
      }\
      CLASS &operator=(const CLASS &) {return *this;};\
      const Expression expr1,expr2;\
  };
//!   Macro for generating a Query that takes three Expressions as arguments.
#define EXPR_THREE_EXPR_CLASS(CLASS,METHOD) \
  class CLASS : public Query {\
    public:\
      CLASS(const Expression &e1,const Expression &e2,const Expression &e3) : expr1(e1), expr2(e2),expr3(e3) {\
      }\
      virtual Result eval(EA3Base *b) const {\
  METHOD;\
  }\
      virtual Query* copy() const {return new CLASS(expr1,expr2,expr3);}\
      CLASS(const CLASS &) {\
      }\
      CLASS &operator=(const CLASS &) {return *this;};\
      const Expression expr1,expr2,expr3;\
  };
//!   Macro for generating a Query that takes three Expressions as arguments.
#define EXPR_THREE_EXPR_CLASS_EXPORT(EX,CLASS) \
  class EX CLASS : public Query {\
    public:\
      CLASS(const Expression &e1,const Expression &e2,const Expression &e3) : expr1(e1), expr2(e2),expr3(e3) {\
      }\
      virtual Result eval(EA3Base *b) const;\
      virtual Query* copy() const {return new CLASS(expr1,expr2,expr3);}\
      CLASS(const CLASS &) {\
      }\
      CLASS &operator=(const CLASS &) {return *this;};\
      const Expression expr1,expr2,expr3;\
  };
//!   Macro for generating a Query that takes four Expressions as arguments.
#define EXPR_FOUR_EXPR_CLASS(CLASS,METHOD) \
class CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4) {\
	}\
	virtual Result eval(EA3Base *b) const {\
	METHOD;\
	}\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4;\
};
//!   Macro for generating a Query that takes four Expressions as arguments.
#define EXPR_FOUR_EXPR_CLASS_EXPORT(EX,CLASS) \
class EX CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4) {\
	}\
	virtual Result eval(EA3Base *b) const;\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4;\
};

//!   Macro for generating a Query that takes five Expressions as arguments.
#define EXPR_FIVE_EXPR_CLASS(CLASS,METHOD) \
class CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4,const Expression &e5) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4),expr5(e5) {\
	}\
	virtual Result eval(EA3Base *b) const {\
	METHOD;\
	}\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4,expr5);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4,expr5;\
};
//!   Macro for generating a Query that takes five Expressions as arguments.
#define EXPR_FIVE_EXPR_CLASS_EXPORT(EX,CLASS) \
class EX CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4,const Expression &e5) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4),expr5(e5) {\
	}\
	virtual Result eval(EA3Base *b) const;\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4,expr5);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4,expr5;\
};
//!   Macro for generating a Query that takes six Expressions as arguments.
#define EXPR_SIX_EXPR_CLASS(CLASS,METHOD) \
class CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4,const Expression &e5,,const Expression &e6) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4),expr5(e5),expr6(e6) {\
	}\
	virtual Result eval(EA3Base *b) const {\
	METHOD;\
	}\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4,expr5,expr6);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4,expr5,expr6;\
};
//!   Macro for generating a Query that takes five Expressions as arguments.
#define EXPR_SIX_EXPR_CLASS_EXPORT(EX,CLASS) \
class EX CLASS : public Query {\
public:\
	CLASS(const Expression &e1,const Expression &e2,const Expression &e3,const Expression &e4,const Expression &e5,const Expression &e6) \
	: expr1(e1),expr2(e2),expr3(e3),expr4(e4),expr5(e5),expr6(e6) {\
	}\
	virtual Result eval(EA3Base *b) const;\
	virtual Query* copy() const {return new CLASS(expr1,expr2,expr3,expr4,expr5,expr6);}\
	CLASS(const CLASS &) {\
	}\
	CLASS &operator=(const CLASS &) {return *this;};\
	const Expression expr1,expr2,expr3,expr4,expr5,expr6;\
};

  class EA3_EXPORT CastableResult : public Result {
  public:
    CastableResult(Result r) : Result(r) {
    }
    operator float() const {
      return toFloat();
    }
    operator double() const {
      return (double)toFloat();
    }
    operator int() const {
      return (int)toInt();
    }
    operator unsigned int() const {
      return (unsigned int)toInt();
    }
    operator const char *() const {
      if (type == RS_NAME || type == RS_TYPE) {
        return data.str;
      }
      return ("?"); //return ? if we passed in a numeric value, this is so the string functions don't crash
    }
  };
  // A Query class that can wrap a function that returns type RT and takes one argument AT
  template <class RT,class AT>
  class DoFunc1 : public Query {
    typedef RT Func(AT);
    public:
      //! Construct based on a single type 
      DoFunc1(Func *f,const Expression &e) : func(f), expr(e)  {
      }
      virtual Result eval(EA3Base *b) const {
        CastableResult r = expr.eval(b);
        return (*func)(r);
      }
      virtual Query* copy() const {
        return new DoFunc1(func,expr);
      }
    //private:
      DoFunc1(const DoFunc1 &other) : func(other.func), expr(other.expr) {}
      DoFunc1 &operator=(const DoFunc1 &) {return *this;}
      Func * func;
      const Expression expr;
  };
  // A Query class that can wrap a function that returns type RT and takes two arguments AT1 and AT2
  template <class RT,class AT1,class AT2>
  class DoFunc2 : public Query {
    typedef RT Func(AT1,AT2);
    public:
      //! Construct based on a two type s
      DoFunc2(Func *f,const Expression &e1, const Expression &e2) : func(f), expr1(e1), expr2(e2)  {
      }
      virtual Result eval(EA3Base *b) const {
        CastableResult r1 = expr1.eval(b);
        CastableResult r2 = expr2.eval(b);
        return (*func)(r1,r2);
      }
      virtual Query* copy() const {
        return new DoFunc2(func,expr1,expr2);
      }
    //private:
      DoFunc2(const DoFunc2 &other) : func(other.func), expr1(other.expr1), expr2(other.expr2) {}
      DoFunc2 &operator=(const DoFunc2 &) {return *this;}
      Func * func;
      const Expression expr1,expr2;
  };
  // A Query class that can wrap a function that returns type RT and takes three arguments AT1, AT2 and AT3
  template <class RT,class AT1,class AT2,class AT3>
  class DoFunc3 : public Query {
    typedef RT Func(AT1,AT2,AT3);
    public:
      //! Construct based on a two type s
      DoFunc3(Func *f,const Expression &e1, const Expression &e2, const Expression &e3) : func(f), expr1(e1), expr2(e2), expr3(e3)  {
      }
      virtual Result eval(EA3Base *b) const {
        CastableResult r1 = expr1.eval(b);
        CastableResult r2 = expr2.eval(b);
        CastableResult r3 = expr3.eval(b);
        return (*func)(r1,r2,r3);
      }
      virtual Query* copy() const {
        return new DoFunc3(func,expr1,expr2,expr3);
      }
    //private:
      DoFunc3(const DoFunc3 &other) : func(other.func), expr1(other.expr1), expr2(other.expr2), expr3(other.expr3) {}
      DoFunc3 &operator=(const DoFunc3 &) {return *this;}
      Func * func;
      const Expression expr1,expr2,expr3;
  };
	/*! 
    perform abs
 
  */
  inline Expression Abs(const Expression &e) {
    return DoFunc1<int, int>(abs,e);
  }
	/*! 
    perform fabs
 
  */
	inline Expression Fabs(const Expression &e) {
    return DoFunc1<double,double>(fabs,e);
  }
  /*! 
    perform sinf
 
  */
  inline Expression Sin(const Expression &e) {
    return DoFunc1<float,float>(sinf,e);
  }
	/*! 
    perform sinh
 
  */
  inline Expression Sinh(const Expression &e) {
    return DoFunc1<double,double>(sinh,e);
  }
	/*! 
    perform asinf
 
  */
  inline Expression Asin(const Expression &e) {
    return DoFunc1<float,float>(sinf,e);
  }
  /*! 
    perform cosf
 
  */
  inline Expression Cos(const Expression &e) {
    return DoFunc1<float,float>(cosf,e);
  }
	/*! 
    perform cosh
 
  */
  inline Expression Cosh(const Expression &e) {
    return DoFunc1<double,double>(cosh,e);
  }
	/*! 
    perform acos
 
  */
	inline Expression Acos(const Expression &e) {
    return DoFunc1<float,float>(acosf,e);
  }
	/*! 
    perform tan
 
  */
	inline Expression Tan(const Expression &e) {
    return DoFunc1<float,float>(tanf,e);
  }
	/*! 
    perform atan
 
  */
	inline Expression Atan(const Expression &e) {
    return DoFunc1<float,float>(atanf,e);
  }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif // unreferenced formal parameter

  class EA3_EXPORT RandomQ : public Query {
    public:
      //! Construct based on a single type 
      RandomQ() {
      }
      virtual Result eval(EA3Base *b) const {
        return (float(rand())/float(RAND_MAX));
      }
      virtual Query* copy() const {
        return new RandomQ();
      }
    //private:
      RandomQ(const RandomQ &other) {}
      RandomQ &operator=(const RandomQ &) {return *this;}
  };

#ifdef _MSC_VER
#pragma warning(pop)
#endif // unreferenced formal parameter

  /*! 
    perform random
 
  */
  inline Expression Random() {
    return RandomQ();
  }
  /*! 
    perform atan2f
 
  */
  inline Expression Atan2(const Expression &e1,const Expression &e2) {
    return DoFunc2<float,float,float>(atan2f,e1,e2);
  }
  /*! 
    perform powf
 
  */
  inline Expression Pow(const Expression &e1,const Expression &e2) {
    return DoFunc2<float,float,float>(powf,e1,e2);
  }
  static unsigned int mystrlen(const char* theString) {
    if (theString[0] == '?')
      return(0);
    else
      return (static_cast<unsigned int>(strlen(theString)));
	}
  /*! 
    perform strlen
 
  */
  inline Expression Strlen(const Expression &e) {
    return DoFunc1<unsigned int,const char *>(mystrlen,e);
  }
	/*! 
    perform atoi
 
  */
  inline Expression Atoi(const Expression &e) {
    return DoFunc1<int,const char *>(atoi,e);
  }
	/*! 
    perform atoi
 
  */
  inline Expression Atol(const Expression &e) {
    return DoFunc1<long,const char *>(atol,e);
  }
	/*! 
    perform atof
 
  */
  inline Expression Atof(const Expression &e) {
    return DoFunc1<double,const char *>(atof,e);
  }
	/*! 
    perform ceil
 
  */
  inline Expression Ceil(const Expression &e) {
    return DoFunc1<double,double>(ceil,e);
  }
	/*! 
    perform exp
 
  */
  inline Expression Exp(const Expression &e) {
    return DoFunc1<double,double>(exp,e);
  }
	/*! 
    perform floor
 
  */
  inline Expression Floor(const Expression &e) {
    return DoFunc1<double,double>(floor,e);
  }
	/*! 
    perform fmod
 
  */
  inline Expression Fmod(const Expression &e1,const Expression &e2) {
    return DoFunc2<double,double,double>(fmod,e1,e2);
  }
	/*! 
    perform ldexp
 
  */
  inline Expression Ldexp(const Expression &e1,const Expression &e2) {
    return DoFunc2<double,double,int>(ldexp,e1,e2);
  }
	/*! 
    perform log
 
  */
  inline Expression Log(const Expression &e) {
    return DoFunc1<double,double>(log,e);
  }
	/*! 
    perform log10
 
  */
  inline Expression Log10(const Expression &e) {
    return DoFunc1<double,double>(log10,e);
  }
	/*! 
    perform sqrt
 
  */
  inline Expression Sqrt(const Expression &e) {
    return DoFunc1<double,double>(sqrt,e);
  }
	inline char* mystrstr(const char* theString,const char *theCharacter) {
        static char emptyString[] = "";
		char *result = const_cast<char*>(strstr(theString,theCharacter));
		if (result == NULL)
			return emptyString;
		else 
			return result;
	}
	/*! 
    perform strstr
 
  */
  inline Expression Strstr(const Expression &e1,const Expression &e2) {
    return DoFunc2<char*,const char*,const char*>(mystrstr,e1,e2);
  }
	inline char* mystrchr(const char* theString,int theCharacter) {
        static char emptyString[] = "";
		char *result = const_cast<char*>(strchr(theString,theCharacter));
		if (result == NULL)
			return emptyString;
		else 
			return result;
	}
	/*! 
    perform strchr
 
  */
  inline Expression Strchr(const Expression &e1,const Expression &e2) {
    return DoFunc2<char*,const char*,int>(mystrchr,e1,e2);
  }
  /*! 
    perform strcmp
 
  */
  inline Expression Strcmp(const Expression &e1,const Expression &e2) {
    return DoFunc2<int,const char*,const char*>(strcmp,e1,e2);
  }
	inline char* mystrrchr(const char* theString,int theCharacter) {
        static char emptyString[] = "";
		char *result = const_cast<char*>(strrchr(theString,theCharacter));
		if (result == NULL)
			return emptyString;
		else 
			return result;
	}
  inline const char *myToLower(const char *str) {
    if (!str) return("");
    XbString s(str);
    return( s.tolower().getString() );
  }
  inline const char *myToUpper(const char *str) {
    if (!str) return("");
    size_t len = strlen(str) + 1;
    char *tl = (char *)EA3Malloc(len);
    for(size_t i=0;i<len;i++) {
      tl[i] = (char)::toupper(str[i]);
    }
    XbString n(tl);
    EA3Free(tl);
    return(n.getString());
  }
  inline int mystrncmp(const char* str1, const char* str2, unsigned int len) {
      return strncmp(str1, str2, (size_t)len);
  }
	/*! 
    perform strrchr
 
  */
  inline Expression Strrchr(const Expression &e1,const Expression &e2) {
    return DoFunc2<char*,const char*,int>(mystrrchr,e1,e2);
  }
  /*! 
    perform strncmp
 
  */
  inline Expression Strncmp(const Expression &e1,const Expression &e2,const Expression &e3) {
    return DoFunc3<int,const char*,const char*,unsigned int>(mystrncmp,e1,e2,e3);
  }
  /*!
    convert string to lower case
 
  */
  inline Expression LowerCase(const Expression &e1) {
    return DoFunc1<const char*,const char*>(myToLower,e1);
  }
  /*!
    convert string to upper case
 
  */
  inline Expression UpperCase(const Expression &e1) {
    return DoFunc1<const char*,const char*>(myToUpper,e1);
  }
  
  inline float myMax(float i1,float i2) {
    if (i1 > i2) return i1;
    else return i2;
  }
  /*!
    returns the larger of its two arguments 
 
  */
  inline Expression Max(const Expression &e1,const Expression &e2) {
    return DoFunc2<float,float,float>(myMax,e1,e2);
  }

  inline float myMin(float i1, float i2) {
    if (i1 < i2) return i1;
    else return i2;
  }
  /*!
    returns the smaller of its two arguments 
 
  */
  inline Expression Min(const Expression &e1,const Expression &e2) {
    return DoFunc2<float,float,float>(myMin,e1,e2);
  }
#ifdef DOXYGEN
	/*! 
    Strrange.  Returns the given string shrunk to the specified range
 
  */
  Result Strrange(Expression string,Expression from,Expression to);
#else
/*
  EXPR_THREE_EXPR_CLASS(Strrange,{
    Result r = expr1.eval(b);
    Result r1 = expr2.eval(b);
    Result r2 = expr3.eval(b);
    // if the lowRange is greater than the highRange return 0
    if (r1.data.i > r2.data.i)
      return (0);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      int i;
      // put  pointer at the lowRange of the string
      const char *stringPtr = &r.data.str[r1.data.i];
      char *newString = new char[r2.data.i - r1.data.i + 2];
      // copy the shortened string into a new string
      for (i=0; i<=r2.data.i-r1.data.i;i++) {
        newString[i] = *stringPtr;
        stringPtr++;
      }
      newString[i] = '\0'; 
      // construct a new XbString with the shortened string
      XbString result;
      result.setString(newString);
      delete [] newString;
      // return the XbString
      return(result);
    }
    return(0);
  });
*/

class Strrange : public Query
{
public:
    Strrange(const Expression &e1,const Expression &e2,const Expression &e3)
        : expr1(e1), expr2(e2),expr3(e3)
    {
    }
    virtual Result eval(EA3Base *b) const
    {
        Result r = expr1.eval(b);
        Result r1 = expr2.eval(b);
        Result r2 = expr3.eval(b);
        // if the lowRange is greater than the highRange return 0
        if (r1.data.i > r2.data.i)
        {
            return 0;
        }
        if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE)
        {
            // put  pointer at the lowRange of the string
            const char *stringPtr = &r.data.str[r1.data.i];
            intptr_t size = r2.data.i - r1.data.i;
            char *newString = new char[static_cast<unsigned int>(size + 2)];
            // copy the shortened string into a new string
            int pos = 0;
            for (pos = 0; pos <= size; ++pos)
            {
                newString[pos] = *stringPtr;
                stringPtr++;
            }
            newString[pos] = '\0'; 
            // construct a new XbString with the shortened string
            XbString result;
            result.setString(newString);
            delete [] newString;
            // return the XbString
            return(result);
        }
        return 0;
    }

    virtual Query* copy() const
    {
        return new Strrange(expr1,expr2,expr3);
    }
private:
    Strrange(const Strrange &)
    {
    }
    Strrange &operator=(const Strrange &)
    {
        return *this;
    };
    const Expression expr1,expr2,expr3;
};


#endif
#ifdef DOXYGEN
  /*! 
    Conditional.  Tests cond and evalutates either whenTrue expression or whenFalse expression
 
  */
  Result If(Expression cond,Expression whenTrue,Expression whenFalse);
#else
  EXPR_THREE_EXPR_CLASS(If,{
    Result r1 = expr1.eval(b);
    if (r1.toInt()) {
      return expr2.eval(b);
    } 
    return expr3.eval(b);
  });
#endif
#ifdef DOXYGEN
  /*! 
    While loop.  While cond is true, evaluate body.  Returns 1 when done.
 
  */
  Result While(Expression cond,Expression body);
#else
  EXPR_TWO_EXPR_CLASS(While,{
    Result r1 = expr1.eval(b);
    Result r2(0);
    while(r1.toInt()) {
      r2 = expr2.eval(b);
      r1 = expr1.eval(b);
    } 
    return 1;
  });
#endif
#ifdef DOXYGEN
  /*! 
    Get the File tail (excludes the path)
 
  File tail:  /a/b/c.d ==> c.d
  */
  Result FileTail(Expression &r);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileTail);
#endif

#ifdef DOXYGEN
  /*! 
    Get the File extension (includes the .)
 
  File extension:  /a/b/c.d ==> .d
  */
  Result FileExt(Expression &r);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileExt);
#endif

#ifdef DOXYGEN
/*! 
  Get the File root (excludes the extension)
 
File root:  /a/b/c.d ==> /a/b/c
*/
Result FileRoot(Expression &r);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,FileRoot);
#endif
#ifdef DOXYGEN
  //! item has type t \sa EA3Base::getType
  Result HasType(const Expression &type);
#else
  class EA3_EXPORT HasType : public Query {
    public:
      //! Construct based on a single type 
      HasType(const Expression &t) : type(t) , mlist(NULL) {
      }
      //! Construct based on a list of types
      HasType(const XbPArray<EA3Base *> &l) : type("_") , mlist(&l) {
      }
      //! evaluate whether one of the types match the type of b.
      virtual Result eval(EA3Base *b) const {
        if (mlist) {
          const XbPArray<EA3Base *> &l = *mlist;
          for(int i=0;i<l.getSize();i++) {
            if (l[i]->getType() == b->getType()) return(1);
          }
          return(0);
        }
				Result r = type.eval(b);
				if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
					return r.data.str == b->getType();
				}
        return 0;
      }
      virtual Query* copy() const {
        if (mlist) {
          return new HasType(*mlist);
        }
        return new HasType(type);
      }
    //private:
      HasType(const HasType &other) : type(other.type), mlist(other.mlist) {}
      HasType &operator=(const HasType &) {return *this;}
      const Expression type;
      const XbPArray<EA3Base *> *mlist;
  };
#endif

#ifdef DOXYGEN
  //! item has type t \sa EA3Base::getType
  Result Is(const Expression &type);
#else
  class EA3_EXPORT Is : public Query {
    public:
      //! Construct based on a single type 
      Is(const XbIdentifier &t) : type(t) , mlist(NULL) {
      }
      //! Construct based on a list of types
      Is(const XbPArray<EA3Base *> &l) : type("_") , mlist(&l) {
      }
      //! evaluate whether one of the types match the type of b.
      virtual Result eval(EA3Base *b) const {
        if (mlist) {
          const XbPArray<EA3Base *> &l = *mlist;
          for(int i=0;i<l.getSize();i++) {
            if (l[i]->getType() == b->getType()) return(1);
          }
          return(0);
        }
        return(b->getType() == type);
      }
      virtual Query* copy() const {
        if (mlist) {
          return new Is(*mlist);
        }
        return new Is(type);
      }
    private:
      Is(const Is &other) : type(other.type), mlist(other.mlist) {}
      Is &operator=(const Is &) {return *this;}
      const XbIdentifier type;
      const XbPArray<EA3Base *> *mlist;
  };
#endif
#ifdef DOXYGEN
   /*!
      Query if name (EA3Base::getName()) matches. 
    This is a very simple query that just compares if an EA3Base::getName() matches the name
    given.
    \sa EA3Expr::GetName and EA3Expr::StringMatch
    \code 
    find(scene,mylist,HasName("LThigh"));
    \endcode
  */  
  Result HasName(const Expression &name);
#else
  class EA3_EXPORT HasName : public Query {
    public:
      //! Construct based on a single name
      HasName(const Expression &n) : name(n) , mlist(NULL) {
      }
      //! Construct based on a list of names
      HasName(const XbPArray<EA3Base *> &l) : name("_") , mlist(&l) {
      }
      //! Evaluate whether one of the names match the name of b.
      virtual Result eval(EA3Base *b) const {
        if (mlist) {
          const XbPArray<EA3Base *> &l = *mlist;
          for(int i=0;i<l.getSize();i++) {
            if (l[i]->getName() == b->getName()) return(1);
          }
          return(0);
        }
				Result nameR = name.eval(b);
				if (nameR.type == Result::RS_NAME || nameR.type == Result::RS_TYPE) {
					return b->getName() == nameR.data.str;
				}
				return 0;
      }
      virtual Query* copy() const {
        if (mlist) {
          return new HasName(*mlist);
        }
        return new HasName(name);
      }
    //private:
      HasName(const HasName &other) : name(other.name), mlist(other.mlist) {};
      HasName &operator=(const HasName &) {return *this;}
      const Expression name;
      const XbPArray<EA3Base *> *mlist;
  };
#endif
#ifdef DOXYGEN
  /*!  
    Regular expression matching
 
  Compares whether the result of an Expression matches a regular expression.
  \sa regexpMatch
  */
  Result RegExp(const Expression &rexp,const Expression &item);
#else
  class EA3_EXPORT RegExp : public Query {
    public:
      struct RCregexp {
        regexp *re;
        int refCount;
        RCregexp(const char *str) {
          re = regComp(const_cast<char *>(str));
          refCount = 1;
        }
        ~RCregexp() {
          EA3Free(re);
        }
      };
      //! Constructor 
      RegExp(const XbString &rexp,const Expression &e) : expr(e) {
        rcre = new RCregexp(rexp.getString());
      }
      //! Constructor
      RegExp(const char *rexp,const Expression &e) : expr(e) {
        rcre = new RCregexp(rexp);
      }
      virtual ~RegExp() {
        rcre->refCount--;
        if (rcre->refCount == 0) delete rcre;
      }
      // evaluate whether the Result of the Expression applied to b matches the regular expression.
      virtual Result eval(EA3Base *b) const {
        Result r = expr.eval(b);
        if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
          if (regExec(rcre->re,const_cast<char *>(r.data.str),const_cast<char *>(r.data.str))) return(1); 
        }
        return(0);
      }
      virtual Query* copy() const {
        return new RegExp(rcre,expr);
      }
    private:
      RegExp(RCregexp *r,const Expression &e) : rcre(r), expr(e) {
        rcre->refCount++;
      }
      RCregexp *rcre;
      const Expression expr;
      RegExp(const RegExp &) : expr(1) {
      }
      RegExp &operator=(const RegExp &) {return *this;};
  };
#endif
#ifdef DOXYGEN
  /*! 
    Simple string matching
 
  Compares whether the result of an Expression matches a string pattern.
  \sa stringMatch
  */
  Result StringMatch(Expression pattern,Expression string);
#else
  EXPR_TWO_EXPR_CLASS(StringMatch,{
    Result p = expr1.eval(b);
    Result r = expr2.eval(b);
    if ((r.type == Result::RS_NAME || r.type == Result::RS_TYPE) &&
      (p.type == Result::RS_NAME || p.type == Result::RS_TYPE)) {
      if (stringMatch(r.data.str,p.data.str)) return(1); 
    }
    return(0);
  });
#endif
#ifdef DOXYGEN
    /*! 
    Regular Substitution 
 
  Performs regular substitution.  
  \sa regExp
  */
Result  RegSub(const Expression &pattern,const Expression &string, const Expression &with,bool all=true,bool nocase=false);
#else
  class EA3_EXPORT RegSub : public Query {
    public:
      //! Constructor
      RegSub(const Expression &pattern,const Expression &string, const Expression &with,bool all=true,bool nocase=false) : 
          m_pattern(pattern),
          m_string(string),
          m_with(with),
          m_all(all),
          m_nocase(nocase) {
      }
      virtual ~RegSub() {
      }
      virtual Result eval(EA3Base *b) const {
        Result pat = m_pattern.eval(b);
        Result str = m_string.eval(b);
        Result wth = m_with.eval(b);

        if ( (pat.type == Result::RS_NAME || pat.type == Result::RS_TYPE) &&
             (str.type == Result::RS_NAME || str.type == Result::RS_TYPE) &&
             (wth.type == Result::RS_NAME || wth.type == Result::RS_TYPE) )
        {
          int num = 0;
          char *out = regsub(pat.data.str,str.data.str,wth.data.str,&num,m_all,m_nocase);
          if (out) {
            XbString val(out);
            EA3Free(out);
            return(val);
          }
        }
        return(0);
      }
      virtual Query* copy() const {
        return new RegSub(m_pattern,m_string,m_with,m_all,m_nocase);
      }
    private:
      const Expression m_pattern,m_string,m_with;
      const bool m_all,m_nocase;
      RegSub(const RegSub &) : 
          m_pattern(""),
          m_string(""),
          m_with(""),
          m_all(1),
          m_nocase(1) {
      }
      RegSub &operator=(const RegSub &) {return *this;};
  };
#endif
#ifdef DOXYGEN
  /*!
    Return the name (EA3Base::getName())  
 
  */ 
Result GetName();
#else
  EXPR_NO_ARGS_CLASS(GetName,return(b->getName()));
#endif

#ifdef DOXYGEN
  /*!
    Return the type (EA3Base::getType())
 
  */
Result GetType();
#else
  EXPR_NO_ARGS_CLASS(GetType,return(b->getType()));
#endif
#ifdef DOXYGEN
  /*!
    Return the reference count.  Will be == 1 if nothing has a reference to it
 
  */
  Result GetRefCount();
#else
  EXPR_NO_ARGS_CLASS(GetRefCount,return(b->refCount()));
#endif
#ifdef DOXYGEN
  /*!
    Set the name (EA3Base::setName())  
 
  */
Result SetName(Expression &r);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,SetName);
#endif
#ifdef DOXYGEN
  /*!
    Print the result the evaluated expression (no new line character)
 
  */
Result Print(Expression &e);
#else
EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,Print);
#endif

#ifdef DOXYGEN
  /*!
    Print the result the evaluated expression (followed by the new line character)
 
  */
Result Printl(Expression &e);
#else
EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,Printl);
#endif

#ifdef DOXYGEN
  /*!
    Convert any result to a string
 
  */
Result ToString(Expression &e);
#else
EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,ToString);
#endif
#ifdef DOXYGEN
  //! convert result to string using a format - like printf.
  Result Format(Expression &format,Expression &expr);
#else
EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,Format);
#endif
// A reference counted result (used in Variables and ArrayIndexers)
struct RCResult {
  Result res;
  int refCount;
  RCResult() {
    refCount = 1;
  }
  RCResult(const Result &_r) : res(_r) {
    refCount = 1;
  }
};
  /*!
    A Variable holds the result of an expression (Result class)
 
  You must put () around any = operators.  Ex:  (X=GetValue(0))
  Example to find the maximum x coordinate value:
  \code
  Variable X(-10000000.0f);
  find(scene,list,IsField() && HasType("coord") && (GetValue(0) > X) && (X=GetValue(0) );
  \endcode
  The last item in the list will be the coordinate with maximum x value.
  */
  
  class EA3_EXPORT Variable : public Query {
    public:
      Variable() {
        r = new RCResult;
        assignment = false;
      }
      virtual ~Variable() {
        r->refCount--;
        if (r->refCount==0) delete r;
      }
      Variable(const Result &r2) {
        r = new RCResult(r2);
        assignment = false;
      }
      Expression operator=(const Expression &e) {
        Variable v2(*this);
        v2.assignment = true;
        v2.expr = e;
        return Expression(v2);
      }
      virtual Result eval(EA3Base *b) const {
        if (assignment) {
          r->res = expr.eval(b);
          return(r->res);
        }
        return(r->res);   
      }
			Result *getResult() {
				return &r->res;
			}
      virtual Query* copy() const {
        return new Variable(*this);
      }
    protected:
      Expression expr; // expression to use for assignment or checking
      bool assignment;
      RCResult *r;
      Variable(const Variable &other) {
        *this = other;
      };
      Variable &operator=(const Variable &other) {
        r = other.r;
        r->refCount++;
        expr = other.expr;
        assignment = other.assignment;
        return *this;
      };
  };
  class InternalResultMap;
  class EA3_EXPORT Result2ResultMap {
  public:
		Result2ResultMap();
		void ref() {
			refCount++;
		}
		void unref() {
			refCount--;
			if (refCount == 0) {
				delete this;
			}
		}
    Result *getResult(const Result &key);
		Result *setResult(const Result &key,const Result &value);
    InternalResultMap *map;
		friend class InternalResultMap;
	private:
		~Result2ResultMap();
		int refCount;
  };


  class EA3_EXPORT ArrayIndexer : public Query {
    public:
      ArrayIndexer(const Expression &e,Result2ResultMap *m) {
        index = e;
        map = m;
				map->ref();
        assignment = false;
      }
      virtual ~ArrayIndexer() {
				map->unref();
      }
      Expression operator=(const Expression &e) {
        ArrayIndexer v2(*this);
        v2.assignment = true;
        v2.expr = e;
        return Expression(v2);
      }
      virtual Result eval(EA3Base *b) const {
				Result ix = index.eval(b);
        Result *res = NULL;
        if (assignment) {
					res = map->setResult(ix,expr.eval(b));
          return(*res);
        }
				res = map->getResult(ix);
				if (res) {
					return *res;
				} 
				return Result();
      }
      virtual Query* copy() const {
        return new ArrayIndexer(*this);
      }
      ArrayIndexer(const ArrayIndexer &other) {
        assignment = other.assignment;
        expr = other.expr;
        index = other.index;
        map = other.map;
				map->ref();
      }
    private:
      Expression expr,index;
      bool assignment;
      Result2ResultMap *map;
      ArrayIndexer &operator=(const ArrayIndexer &) {
        return *this;
      };
      
      friend class ResultsMap;
  };
  /*!
    Like variables but can use an expression as a key into a map.
 
  Example:  Find unique filenames of textures
  \code
  void uniqueFilenames(EA3Scene *s,XbPArray<EA3Base *> &mlist) {
    ResultsMap RM;
    find(s,mlist,HasType("filename")&&IsField()&&(RM[GetValue(0)]=1));
    for(ResultsMap::Iterator i=RM.begin();i!=RM.end();++i) {
      Result x = *i.value();
      Result y = i.key();
      printf("filename = %s\n",y.data.str);
    }
  }
  \endcode
  The last item in the list will be the coordinate with maximum x value.
  */
  class EA3_EXPORT ResultsMap {
  public:
    class EA3_EXPORT Iterator {
    public:
      Result *value() const;
      Result key() const;
      Iterator &operator++();
      bool operator==(const Iterator &other) const;
      bool operator!=(const Iterator &other) const {
        return !(*this==other);
      }
			
    private:
      int internal[4];
      friend class ResultsMap;
    };
		int getSize() const;
    Iterator begin();
    Iterator end();
		Result *getResult(const Result &key);
		Result *setResult(const Result &key,const Result &value);
    ResultsMap() {
			map = new Result2ResultMap;
    }
		ResultsMap(const ResultsMap &other) {
			map = other.map;
			map->ref();
		}
		
    ~ResultsMap() {
			map->unref();
    }
    ArrayIndexer operator[] (const Expression &e) {
      return(ArrayIndexer(e,map));
    }
		void clear();
  private:
		ResultsMap &operator=(const ResultsMap &other) {
			return *this;
		}
    Result2ResultMap *map;
  };
#ifdef DOXYGEN    
  /*! 
    Iterate over each element in a ResultsMap
 
  Examples:
  \code
  ResultsMap RM;
  find(scene,list,HasName(foo)&&RM[This()]=GetName()));
  Variable I;
  ForEachMapIndex(I,&RM,Print(RM[I]).eval(NULL);
  \endcode
  */
  Result ForEachMapIndex(Varible index,ResultsMap *,Expression to_do);
#else
  EXPR_THREE_EXPR_CLASS_EXPORT(EA3_EXPORT,ForEachMapIndex);
#endif
#ifdef DOXYGEN
  /*!  
      Query if IsSelected() (EA3IsSelected(EA3Base *b)) 
  */
  Result IsSelected();
#else
  EXPR_NO_ARGS_CLASS(IsSelected,return(EA3IsSelected(b)););
#endif

#ifdef DOXYGEN
  /*!
      Query if Select() (EA3Select(EA3Base *b,true)) 
  */
  Result Select();
#else
  EXPR_NO_ARGS_CLASS(Select,{EA3SetSelect(b,true);return(1);});
#endif
#ifdef DOXYGEN
  /*!
      Query if UnSelect() (EA3Select(EA3Base *b,false)) 
 
  */
  Result UnSelect();
#else
  EXPR_NO_ARGS_CLASS(UnSelect,{EA3SetSelect(b,false);return(1);});
#endif

#ifdef DOXYGEN
  /*!
      Query if IsArray() (EA3Base::isArray()) 
 
  */
  Result IsArray();

#else
  EXPR_NO_ARGS_CLASS(IsArray,return(b->isArray()));
#endif

#ifdef DOXYGEN
  /*! Result UnSelect();
      Query if IsBlock() (EA3Base::isBlock() && !EA3::isArray())
 
  */
  Result UnSelect();
#else
  EXPR_NO_ARGS_CLASS(IsBlock,return(b->isBlock() && !b->isArray()));
#endif

#ifdef DOXYGEN
  /*! 
      Query if IsField() (EA3Base::isField())
 
  */
  Result IsField();
#else
  EXPR_NO_ARGS_CLASS(IsField,return(b->isField()));
#endif

#ifdef DOXYGEN
  /*! Result IsReference();
      Query if IsReference() (EA3Base::isReference())
 
  */
  Result IsReference();
#else
  EXPR_NO_ARGS_CLASS(IsReference,return(b->isReference()));
#endif

#ifdef DOXYGEN
  /*! Result IsArrayOfFields();
      Returns true if and only if item is an array and all its members are fields.
 
  */
  Result IsArrayOfFields();
#else
  EXPR_NO_ARGS_CLASS(IsArrayOfFields,return(b->isArray()&&b->toArray()->getNumMembers()>0&&b->toArray()->getByIndex(0)->isField()));
#endif

#ifdef DOXYGEN
  /*!
      Query, return the current EA3Base pointer.  Usually used to set or compare with a Variable.
 
  */
  Result This();
#else
  EXPR_NO_ARGS_CLASS(This,return(b));
#endif

#ifdef DOXYGEN
  /*!  
      Check if a field has the value given.  Multiple forms are possible.
 
    \code
    HasValue("trn");
    float f[3] = {0.0f,0.0f,0.0f};
    HasValue(f,3);
    \endcode
  */
  Result HasValue(const Expression &value);
#else
  class EA3_EXPORT HasValue : public Query {
    public:
      //! constructor
      HasValue(const float * const f, int n) : pFloat(f) , pInt(NULL) , num(static_cast<unsigned int>(n))  {
      }
      //! constructor
      HasValue(const int * const i, int n) : pFloat(NULL), pInt(i) ,num(static_cast<unsigned int>(n)) {
      }
      //! constructor
      HasValue(const Expression &ex) : pFloat(NULL), pInt(NULL) ,expr(ex), num(1) {
      }
      virtual Result eval(EA3Base *b) const {
        if (!b) return(0);
        EA3Field *f = b->toField();
        if (f && f->getNum() == num) {
          int equal = 1;
          if (pFloat) {
            for(unsigned int i=0;i<num;i++) {
              if(f->data[i].f != pFloat[i]) {
                equal = 0;
                break;
              }
            }
          } else if (pInt) {
            for(unsigned int i=0;i<num;i++) {
              if(f->data[i].i != pInt[i]) {
                equal = 0;
                break;
              }
            }
          } else  {
						Result r = expr.eval(b);
						char dt = f->getDataType(0);
						if (r.type == Result::RS_FLOAT && dt == 'f') return r.data.f == f->data[0].f;
						if (r.type == Result::RS_INT && dt == 'i') return r.data.i == f->data[0].i;
            if ((r.type == Result::RS_NAME || r.type == Result::RS_TYPE) && (dt == 's' || dt=='d')) return r.data.str == f->data[0].bindata;
            return(0);
          }
          return(equal);
        }
        return(0);
      }
      virtual Query* copy() const {return new HasValue(*this);}
    //private:
      HasValue(const HasValue &other) : pFloat(other.pFloat), pInt(other.pInt), expr(other.expr), num(other.num) {
      }
      HasValue &operator=(const HasValue &) {return *this;};
      const float * const pFloat;
      const int * const pInt;
      const Expression expr;
      const unsigned int num;
  };
#endif

#ifdef DOXYGEN
  /*! 
    Get the value of a field at index i
 
  Examples:
  \code
  find(scene,mylist,GetValue(0) > 10.0f);
  find(scene,mylist,( (GetValue(0) + GetValue(1) + GetValue(2) ) > 100.0f) );
  \endcode
  */
  Result GetValue(Expression index)
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,GetValue);
#endif

#ifdef DOXYGEN
  /*! 
    Get the data type of a field at index i
 
  Examples:
  \code
  find(scene,mylist,GetDataType(0) == "s");
  \endcode
  */
  Result GetDataType(Expression index)
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,GetDataType);
#endif

#ifdef DOXYGEN
  /*! 
    Get the data types of a field
 
  Examples:
  \code
  find(scene,mylist,GetDataTypes() == "fff");
  \endcode
  */
  Result GetDataTypes(Expression index)
#else
  EXPR_NO_ARGS_CLASS(GetDataTypes,return(b->isField()? b->toField()->getDataType() : "");)
#endif


#ifdef DOXYGEN
	/*! 
    Get the index of the specified block
 
  Example:
  \code
  find(scene,mylist,GetIndex() == 1);
  \endcode
  */
  Result GetIndex();
#else
  EXPR_NO_ARGS_CLASS(GetIndex,return(b->getIndex()););
#endif

#ifdef DOXYGEN
/*!  Result SetExtraTypes(Expression &e);
    Set the extra value types for a field.  
  See EA3Field::setExtraTypes
 
*/
  Result SetExtraTypes(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,SetExtraTypes);
#endif

#ifdef DOXYGEN
  /*! Result GetExtraTypes(Expression &e);
    Get the extra value types for a field.  
  See EA3Field::getExtraTypes
 
  */
  Result GetExtraTypes();
#else
  EXPR_NO_ARGS_CLASS(GetExtraTypes,{
    EA3Field *f = b->toField();
    if (!f) return("");
    const char *et = f->getExtraTypes();
    if (!et) return("");
    return(et);
  });
#endif
#ifdef DOXYGEN
/*!  \fn Result GetNumData();
    Get the number of data types for a field.  
  See EA3Field::getNum
 
*/ 
  Result GetNumData();
#else
  EXPR_NO_ARGS_CLASS(GetNumData,{
    EA3Field *f = b->toField();
    if (!f) return(0);
    return((int)f->getNum());
  };);
#endif
#ifdef DOXYGEN
/*!  \fn Result GetNumExtraTypes();
    Get the number of extra types for a field.  
  See EA3Field::getNumExtraTypes
 
*/ 
  Result GetNumExtraTypes();
#else
  EXPR_NO_ARGS_CLASS(GetNumExtraTypes,{
    EA3Field *f = b->toField();
    if (!f) return(0);
    return((int)f->getNumExtraTypes());
  };);
#endif     
#ifdef DOXYGEN
/*! 
    Get the extra value of a field at index i
 
  See EA3Field::setExtraTypes
  Examples:
  \code
  find(scene,mylist,GetXValue(0) > 10.0f);
  find(scene,mylist,( (GetXValue(0) + GetValue(1) + GetValue(2) ) > 100.0f) );
  \endcode
*/
  Result GetExtraValue(Expression index);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,GetExtraValue);
#endif
#ifdef DOXYGEN   
  /*! 
    Set the value of a field at index i with result of expression.  Returns 1 if value is set, 0 otherwise
 
  Examples:
  \code
  find(scene,mylist,GetValue(0) > 10.0f && SetValue(0,10.0));
  \endcode
  */
  Result SetValue(Expression index,Expression value);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,SetValue);
#endif
#ifdef DOXYGEN 
   /*!  
      Like HasMember, but can take two arguments, first narrows search to only members of that type or with integer index.  Works on EA3Array's, EA3Block's and EA3References
 
    \code
    Has(material,Has(diffuse,GetValue(3)<1.0))
    I=0;
    Node=Has(I,1); // same as EA3Block::getByIndex(I) or EA3Reference::getReference(I);
    \endcode
  */
  Result Has(Expression type,Expression match);
  
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,_Has);
#endif
  /*!  
      Like HasMember, Works on EA3Array's, EA3Block's and EA3References
 
    \code
    Has(GetValue(3)<1.0)
    \endcode
  */
  inline Expression Has(Expression &match) {
    return _Has(-1,match);
  }
  inline Expression Has(Expression &type,Expression &match) {
    return _Has(type,match);
  }
#ifdef DOXYGEN
  /*! 
    Set the extra value of a field at index i with result of expression
 
  See EA3Field::setExtraTypes
  Examples:
  \code
  find(scene,mylist,GetExtraValue(0) == "Frog" && SetExtraValue(0,"Toad"));
  \endcode
  */
  Result SetExtraValue(Expression index,Expression value);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,SetExtraValue);
#endif  

#ifdef DOXYGEN
	/*! 
    Sets a reference by index
 
  Examples:
  \code
  Var B = This(); SetReference(0,B)
  \endcode
  */
  Result SetReference(Expression index,Expression basePtr);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,SetReference);
#endif
#ifdef DOXYGEN
  /*! 
    Inserts a reference at index
 
  Examples:
  \code
  Var B = This(); InsertReference(0,B)
  \endcode*/
  Result InsertReference(Expression index,Expression basePtr);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,InsertReference);
#endif

#ifdef DOXYGEN
  /*! 
    Adds a reference
 
  Examples:
  \code
  Var B = This(); AddReference(B)
  \endcode*/
  Result AddReference(Expression basePtr);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,AddReference);
#endif

#ifdef DOXYGEN
  /*! 
    Removes a reference by index or EA3Base pointer
 
  Examples:
  \code
  Var B = This(); RemoveReference(B)
  \endcode*/
  Result RemoveReference(Expression ptrOrIndex);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,RemoveReference);
#endif

#ifdef DOXYGEN
  /*! 
      Query that returns the first item where Expression evaluates to non-zero (could be current item)
 
  */
  Result Find(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,Find);
#endif

#ifdef DOXYGEN
  /*!  
      Query if item has a member (not recursive) that matches expression
 
  */
  Result HasMember(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,HasMember);
#endif

#ifdef DOXYGEN
  /*!  
      Return the container if it exists and matches the expression
 
  */
  Result HasContainer(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,HasContainer);
#endif

#ifdef DOXYGEN
  /*!  
      create a field.  Evaluates the second expression with the new field.
    Returns the field if successful.
 
  */
  Result CreateField(Expression &e1,Expression &e2);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,CreateField);
#endif
#ifdef DOXYGEN
  /*!  
      create a Block.  Evaluates the second expression with the new Block.
    Returns the Block if successful.
 
  */
  Result CreateBlock(Expression &e1,Expression &e2);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,CreateBlock);
#endif

#ifdef DOXYGEN
  /*!  
      create a Array.  Evaluates the second expression with the new Array.
    Returns the Array if successful.
 
  */
  Result CreateArray(Expression &e1,Expression &e2);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,CreateArray);
#endif

#ifdef DOXYGEN
  /*!  
      create a Reference.  Evaluates the second expression with the new Reference.
    Returns the Reference if successful.
 
  */
  Result CreateReference(Expression &e1,Expression &e2);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,CreateReference);
#endif

#ifdef DOXYGEN
  /*!  
      If the result of the first expression is an EA3Base, perform the second expression on it.
 
  */
  Result With(Expression &e1,Expression &e2);
#else
  EXPR_TWO_EXPR_CLASS(With,{
    Result r = expr1.eval(b);
    if (r.type == Result::RS_BASE) {
      return(expr2.eval((EA3Base *)r.data.v));
    }
    return 0;
  });
#endif

#ifdef DOXYGEN
  /*! 
    get a container recursively based on expression
 
  Takes two expressions, the first is used to find the container (could be current item) 
  by recursively calling EA3Base::getContainer().
  The second is used if the first expression is satisfied, and is applied to the container.
  Example to find bones nodes of type "geom" that have associated geometry nodes.
  \code
  Variable A;
  find(scene,list,
    HasType("bone") && // bone block
    HasMember(HasType("type") && HasValue("geom")) && // type field is "geom"
    (A=GetName()) &&  // store node name in variable
    GetContainer(
      HasType("model"), // get the container recursively that is of type "model"
      Find(
        HasType("geometry") && !IsArray() && (GetName()==A)  // find the geometry block with name A
      )
    )
  );
  \endcode
  */
  Result GetContainer(Expression e1,Expression e2);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,GetContainer);
#endif    

#ifdef DOXYGEN
  /*! 
      Return the number of members that match the expression (if one is given)
 
  */
  Result NumMember(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,NumMember);
#endif

#ifdef DOXYGEN
  /*! 
      Return the number of references that match an expression (if given)
 
  */
  Result NumReferences(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,NumReferences);
#endif
#ifdef DOXYGEN
  /*! 
      Returns the reference that matches the expression else 0
 
  */
  Result HasReference(Expression &e);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,HasReference);
#endif
  inline Expression::Expression(const Query &q) {
    evaluatorCB = NULL;
    query = q.copy();
    rhs = NULL;
    lhs = NULL;
  }
  
#ifdef DOXYGEN
  //! return count of items found recursively
  Result Count(const Expression &expr);
#else
  EXPR_ONE_EXPR_CLASS( Count,{  
    int ret = 0;
    XbPArray<EA3Base*> selList;
    EA3Block *temp = b->toBlock();

    find(temp,selList,expr);
    ret = selList.getSize();

    return ret;
  });
#endif

#ifdef DOXYGEN
  //! run a system call on result of expression (if a string).
  Result Exec(const Expression &expr);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,Exec);
#endif

#ifdef DOXYGEN
/*!  \fn Result NewMap();
    Return a new Map for mapping keys to values.
		\sa MapSize(map), MapSetValue(map,key,value) and MapGetValue(map,key)
*/ 
  Result NewMap();
#else
  EXPR_NO_ARGS_CLASS(NewMap,{
    return new ResultsMap();
  };);
#endif     

#ifdef DOXYGEN
/*!  \fn Result MapSize(map);
    Return the number of keys in a Map
		\sa NewMap(), MapSetValue(map,key,value),and MapGetValue(map,key)
*/ 
  Result MapSize(Expression map);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,MapSize);

#endif     

#ifdef DOXYGEN
/*!  \fn Result MapGetValue(Expression map,key);
    Return the value at a given key
		\sa NewMap(), MapSetValue(map,key,value) and MapSize(map)
*/ 
  Result MapGetValue(Expression map, Expression key);
#else
  EXPR_TWO_EXPR_CLASS_EXPORT(EA3_EXPORT,MapGetValue);
#endif     

	#ifdef DOXYGEN
/*!  \fn Result MapSetValue(Expression map,key,value);
    Return the value at a given key
		\sa NewMap(), MapGetValue(map,key) and MapSize(map)
*/ 
  Result MapSetValue(Expression map, Expression key,Expression value);
#else
  EXPR_THREE_EXPR_CLASS_EXPORT(EA3_EXPORT,MapSetValue);
#endif

#ifdef DOXYGEN
  /*! 
      Clears all the entries in a map
  */
  Result MapClear(Expression &map);
#else
  EXPR_ONE_EXPR_CLASS_EXPORT(EA3_EXPORT,MapClear);
#endif
  
} // namespace

#endif
