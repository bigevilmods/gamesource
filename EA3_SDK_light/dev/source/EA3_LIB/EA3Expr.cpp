// (c) 1998-2010 Electronic Arts Inc.

#include <eabase/eabase.h>
#include "EA3Expr.h"

#include <eastl/hash_map.h>
using namespace eastl;

// Moved to header and inlined

namespace EA3Expr {
	void Result::clear() {
		if (type == RS_RM_PTR) {
			ResultsMap *rm = static_cast<ResultsMap*>(data.v);
			delete rm;
		} else if (type == RS_MATRIX) {
			EA3HeapFree(data.v,16);
		} else if (type == RS_VEC3) {
			EA3HeapFree(data.v,3);
		}
		type = RS_UNKNOWN;
	}
	Result::~Result() {
		clear();
	}
	Result::Result(ResultsMap *rm) : type(RS_RM_PTR) {
		data.v = static_cast<void *>(new ResultsMap(*rm));
	}
	Result &Result::operator =(const Result &other) {
		if (other.type == RS_RM_PTR) {
			ResultsMap *rm = static_cast<ResultsMap*>(other.data.v);
			ResultsMap *copy = new ResultsMap(*rm);
			data.v = static_cast<void *>(copy);
		} else if (other.type == RS_MATRIX) {
			if (type!=other.type) {
				clear();
				data.v = EA3HeapAlloc(16);
			}
			memcpy(data.v,other.data.v,16*sizeof(float));
		} else if (other.type == RS_VEC3) {
			if (type!=other.type) {
				clear();
				data.v = EA3HeapAlloc(3);
			}
			memcpy(data.v,other.data.v,3*sizeof(float));
		} else if (other.type != RS_UNKNOWN) {
			memcpy(&data,&other.data,sizeof(data));
		}
		type = other.type;
		return (*this);
	}
  struct ResultEqual {
    bool operator()(const Result &r1,const Result &r2) const {
      return((r1==r2).toInt()!=0);
    }
  }; 
  struct ResultHash : public unary_function<const Result &,size_t>  {
    size_t operator()(const Result &r) const {
      switch (r.type) {
        //RS_FLOAT,RS_INT,RS_UINT,RS_NAME,RS_TYPE,RS_BASE,RS_VEC3,RS_MATRIX,RS_UNKNOWN
      case Result::RS_INT: return(r.data.i);
      case Result::RS_UINT: return(r.data.ui);
      case Result::RS_NAME:
      case Result::RS_TYPE: return((size_t)const_cast<char *>(r.data.str));
      case Result::RS_FLOAT: return(*(size_t*)const_cast<float*>(&r.data.f));
      case Result::RS_VEC3: return(*(size_t*)const_cast<float*>(&r.data.v3()[0]));
      case Result::RS_MATRIX: return(*(size_t*)const_cast<float*>(&r.data.m44()[0][0]));
      case Result::RS_BASE: 
			case Result::RS_IMAGE:
			case Result::RS_FILE_PTR:
			case Result::RS_RM_PTR:
				return((size_t)const_cast<void *>(r.data.v));
      default:
        break;
      }
      return((size_t)r.toInt());
    }
  };
  typedef hash_map<Result,Result *,ResultHash,ResultEqual, StlAllocator > Map;
  
  class InternalResultMap {
  public:
    InternalResultMap() {
      hmap=new Map;
    } 
    Result *getResult(const Result &key) {
			Map::iterator mi = hmap->find(key);
      if (mi==hmap->end()) return NULL;
      return (*mi).second;
    }
		Result *setResult(const Result &key,const Result &value) {
			Map::iterator mi = hmap->find(key);
			Result *nr;
			if (mi==hmap->end()) {
				nr = (Result*)EA3HeapAlloc(sizeof(Result)/4);
				nr->type = Result::RS_UNKNOWN;
				(*hmap)[key] = nr;
			} else {
				nr = (*mi).second;
			}
			*nr = value;
			return nr;
		}
    ~InternalResultMap() {
      for(Map::iterator i=hmap->begin();i!=hmap->end();++i) {
        EA3HeapFree((*i).second,sizeof(Result)/4);
      }
      delete hmap;
    }
    Map *hmap;
  };

  
  Result *Result2ResultMap::getResult(Result const &key) {
    return map->getResult(key);
  }
	Result *Result2ResultMap::setResult(Result const &key,Result const &value) {
    return map->setResult(key,value);
  }
  
  Result2ResultMap::Result2ResultMap() {
    map = new InternalResultMap;
		refCount = 1;
  }
  Result2ResultMap::~Result2ResultMap() {
    delete map;
  }
  
	void ResultsMap::clear() {
		map->map->hmap->clear();
	}
  ResultsMap::Iterator ResultsMap::begin() {
    ResultsMap::Iterator i;
    Map::iterator it = map->map->hmap->begin();
    memcpy(i.internal,&it,sizeof(it));
    return i;
  } 
  ResultsMap::Iterator ResultsMap::end() {
    ResultsMap::Iterator i;
    Map::iterator it = map->map->hmap->end();
    memcpy(i.internal,&it,sizeof(it));
    return i;
  }
  ResultsMap::Iterator &ResultsMap::Iterator::operator++() {
    Map::iterator i1;
    memcpy(&i1,internal,sizeof(i1));
    i1++;
    memcpy(internal,&i1,sizeof(i1));
    return *this;
  }
  bool ResultsMap::Iterator::operator==(const ResultsMap::Iterator &other) const {
    Map::iterator i1,i2;
    memcpy(&i1,other.internal,sizeof(i1));
    memcpy(&i2,internal,sizeof(i2));
    return(i1 == i2);
  }
  Result *ResultsMap::Iterator::value() const {
    Map::iterator i1;
    memcpy(&i1,internal,sizeof(i1));
    return (*i1).second;
  }
  Result ResultsMap::Iterator::key() const {
    Map::iterator i1;
    memcpy(&i1,internal,sizeof(i1));
    return (*i1).first;
  }
	int ResultsMap::getSize() const {
		return static_cast<int>(map->map->hmap->size());
	}
	Result *ResultsMap::getResult(const Result &key) {
		return map->getResult(key);
	}
	Result *ResultsMap::setResult(const Result &key,const Result &value) {
		return map->setResult(key,value);
	}
    

  Result ForEachMapIndex::eval(EA3Base *b) const {
    Result r2 = expr2.eval(b);
    if (r2.type != Result::RS_RM_PTR) return false;
    if (!expr1.getQuery()) return(false);
    Variable *v = (Variable *)expr1.getQuery();
    
    ResultsMap *rm = (ResultsMap*)r2.data.v;
    for(ResultsMap::Iterator ri = rm->begin();ri!=rm->end();++ri) {
      Expression E(*v=Expression(ri.key()));
      E.eval(NULL);
      expr3.eval(b);
    }
    return(true);
  }

  Result _FileTail::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      const char *res = r.data.str;
      const char *tmp = strrchr(res,'\\');
      if (tmp) {
        return(tmp+1);
      } 
      tmp = strrchr(res,'/');
      if (tmp) {
        return(tmp+1);
      }
    }
    return(r);
  }
  
  Result _FileExt::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      const char *res = r.data.str;
      const char *tmp = strrchr(res,'.');
      if (tmp) {
        return(tmp);
      }
    }
    return(r);
  }
  Result _FileRoot::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    XbString s = "";
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      char *res = XbString::_strdup(r.data.str);
      char *tmp = strrchr(res,'.');
      if (tmp) {
        *tmp = 0;
      }
      s = res;
      EA3Free(res);
    }
    return(s);
  }
  Result _SetName::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    if (r.type==Result::RS_NAME||r.type==Result::RS_TYPE) {
      b->setName(r.data.str);
    }
    return(1);
  }
  static PrintCB *printCB=NULL;
  static void *PCuserData=NULL;
  void setPrintCallback(PrintCB *pc,void *userData) {
    printCB = pc;
    PCuserData = userData;
  }

	void printResult(Result &r) {
    char buf[512];
    char* temp=NULL;

    if (r.type==Result::RS_TYPE || r.type==Result::RS_NAME) {
      temp=const_cast<char *>(r.data.str);
      //temp=(char*)r.data.str;
      fflush(stdout);
      //flush(stdout);
      //return(1);
    }
    if (r.type==Result::RS_FLOAT) {
      //printf("%f",r.data.f);
      sprintf(buf,"%f",r.data.f); temp=buf;
    }
    if (r.type==Result::RS_INT) {
      //printf("%d",r.data.i);
      sprintf(buf,"%d",(int)r.data.i); temp=buf;
    }
    if (r.type==Result::RS_UINT) {
      //printf("%#x",r.data.ui);
      sprintf(buf,"%#x",(unsigned int)r.data.ui); temp=buf;
    }
    if (r.type == Result::RS_BASE || r.type == Result::RS_RM_PTR) {
      //printf("0x%p",r.data.v);
      sprintf(buf,"0x%p",r.data.v); temp=buf;
    }
    if (r.type==Result::RS_VEC3) {
      //printf("(%f %f %f)",r.data.v3()[0],r.data.v3()[1],r.data.v3()[2]);
      sprintf(buf,"(%f %f %f)",r.data.v3()[0],r.data.v3()[1],r.data.v3()[2]); temp=buf;
    }
    if (r.type==Result::RS_MATRIX) {
      sprintf(buf,"(%f %f %f %f)\n(%f %f %f %f)\n(%f %f %f %f)\n(%f %f %f %f)\n",
        r.data.m44()[0][0],r.data.m44()[0][1],r.data.m44()[0][2],r.data.m44()[0][3],
        r.data.m44()[1][0],r.data.m44()[1][1],r.data.m44()[1][2],r.data.m44()[1][3],
        r.data.m44()[2][0],r.data.m44()[2][1],r.data.m44()[2][2],r.data.m44()[2][3],
        r.data.m44()[3][0],r.data.m44()[3][1],r.data.m44()[3][2],r.data.m44()[3][3]); temp=buf;
      /*printf("(%f %f %f %f)\n",r.data.m44()[0][0],r.data.m44()[0][1],r.data.m44()[0][2],r.data.m44()[0][3]);
      printf("(%f %f %f %f)\n",r.data.m44()[1][0],r.data.m44()[1][1],r.data.m44()[1][2],r.data.m44()[1][3]);
      printf("(%f %f %f %f)\n",r.data.m44()[2][0],r.data.m44()[2][1],r.data.m44()[2][2],r.data.m44()[2][3]);
      printf("(%f %f %f %f)\n",r.data.m44()[3][0],r.data.m44()[3][1],r.data.m44()[3][2],r.data.m44()[3][3]);*/
    }
		if (r.type==Result::RS_UNKNOWN) {
			sprintf(buf,"  *UNKNOWN TYPE*  Probably an error.");
			temp=buf;
		}
    if (temp)
    {
      if (printCB)
        (*printCB)(temp,PCuserData);
      else
        printf("%s",temp);
    }

    fflush(stdout);
  }

	Result _Printl::eval(EA3Base *b) const {
		Result r = expr.eval(b);
		printResult(r);
		if (printCB)
			(*printCB)("\n",PCuserData);
		else
			printf("\n");
		return 1;
	}

  Result _Print::eval(EA3Base *b) const {
    Result r = expr.eval(b);
		printResult(r);
		return 1;
	}
	
  Result _ToString::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    char buf[1000];
    if (r.type==Result::RS_TYPE || r.type==Result::RS_NAME) {
      return(r);
    }
    if (r.type==Result::RS_FLOAT) {
      sprintf(buf,"%f",r.data.f);
    }
    if (r.type==Result::RS_INT) {
      sprintf(buf,"%d",(int)r.data.i);
    }
    if (r.type==Result::RS_UINT) {
      sprintf(buf,"%#x",(unsigned int)r.data.ui);
    }
    if (r.type == Result::RS_BASE || r.type == Result::RS_RM_PTR) {
      sprintf(buf,"0x%p",r.data.v);
    }
    if (r.type==Result::RS_VEC3) {
      sprintf(buf,"(%f %f %f)",r.data.v3()[0],r.data.v3()[1],r.data.v3()[2]);
    }
    if (r.type==Result::RS_MATRIX) {
      sprintf(buf,"(%f %f %f %f):",r.data.m44()[0][0],r.data.m44()[0][1],r.data.m44()[0][2],r.data.m44()[0][3]);
      sprintf(buf,"(%f %f %f %f):",r.data.m44()[1][0],r.data.m44()[1][1],r.data.m44()[1][2],r.data.m44()[1][3]);
      sprintf(buf,"(%f %f %f %f):",r.data.m44()[2][0],r.data.m44()[2][1],r.data.m44()[2][2],r.data.m44()[2][3]);
      sprintf(buf,"(%f %f %f %f)",r.data.m44()[3][0],r.data.m44()[3][1],r.data.m44()[3][2],r.data.m44()[3][3]);
    }
    return(buf);
  }
  Result _SetExtraTypes::eval(EA3Base *b) const {
    Result r = expr.eval(b);
    EA3Field *f = b->toField();
    if (!f) return(0);
    if (r.type==Result::RS_NAME||r.type==Result::RS_TYPE) {
      return(f->setExtraTypes(r.data.str));
    }
    return(0);
  }
  Result _Find::eval(EA3Base *b) const {
    EA3Base *f = findFirst(b,expr);
    if (f) return(f);
    return(0);
  }
  Result _HasMember::eval(EA3Base *b) const {
    if (b->isBlock()) {
      EA3Block *bl = b->toBlock();
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
	  for(int i = 0;it!=it.end()&&i<num;++it,++i) {
        Result r=expr.eval(*it);
        if (r.toInt()) return(r);
      }
    }
    return(0);
  }
  Result _HasContainer::eval(EA3Base *b) const {
    EA3Block *con = b->getContainer();
    if (!con) return(0);
    return (expr.eval(con));
  }
  Result _NumMember::eval(EA3Base *b) const {
    int count = 0;
    if (!b) return(count);
    EA3Block *bl = b->toBlock();
    if (bl) {
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
	  for(int i = 0;it!=it.end()&&i<num;++it, ++i ) {
        if (expr.eval(*it).toInt()) count++;
      }
    }
    return(count);
  }
  Result _NumReferences::eval(EA3Base *b) const {
    int count = 0;
    if (!b) return(count);
    EA3Reference *ref = b->toReference();
    if (ref) {
      int num = ref->getNumReferences();
	  EA3Iterator it = ref->begin();
      for(int i=0;i<num&&it!=it.end();i++,++it) {
        if (expr.eval(*it).toInt()) count++;
      }
    }
    return(count);
  }
  Result _HasReference::eval(EA3Base *b) const {
    if (b->isReference()) {
      EA3Reference *ref = b->toReference();
      int num = ref->getNumReferences();
	  EA3Iterator it = ref->begin();
      for(int i=0;i<num&&it!=it.end();i++,++it) {
        Result r = expr.eval(*it);
        if (r.toInt()) return(r);  
      }
    }
    return(0);
  }
  
  Result _GetValue::eval(EA3Base *b) const {
    unsigned int index = (unsigned int)expr.eval(b).toInt();
    Result r=0;
    if (!b) return(r);
    EA3Field *f = b->toField();
    if (f && f->getNum() > index) {
      char dt = f->getDataType(index);
      switch(dt) {
      case 'f': return(f->data[index].f);
      case 'i': return(f->data[index].i);
      case 'c': return(f->data[index].ui);
      case 's': return(XbString(f->data[index].str));
      case 'n':
      case 'd': return(XbIdentifier(f->data[index].str));
      default: return(r);
      }
    }
    return(r);
  }

  Result _GetDataType::eval(EA3Base *b) const {
    unsigned int index = (unsigned int)expr.eval(b).toInt();
    Result r=0;
    if (!b) return(r);
    EA3Field *f = b->toField();
    if (f && f->getNum() > index) {
      char buf[2] = {0,0};
      buf[0] = f->getDataType(index);
      return(buf);
    }
    return(r);
  }
  
  Result _Exec::eval(EA3Base *b) const {
    int ret = -1;
#ifdef DOXYGEN
    Result r = expr.eval(b);

    if(r.type == Result::RS_NAME){
      ret = system(r.data.str);
    }
#endif
    return ret;
  }

  Result _GetExtraValue::eval(EA3Base *b) const {
    unsigned int index = (unsigned int)expr.eval(b).toInt();
    Result r=0;
    if (!b) return(r);
    EA3Field *f = b->toField();
    if (!f) return(r);
    int num = f->getNum();
    if (f->getNumExtraTypes() > index) {
      const char *et = f->getExtraTypes();
      char dt = et[index];
      switch(dt) {
      case 'f': return(f->data[num+index].f);
      case 'i': return(f->data[num+index].i);
      case 'c': return(f->data[num+index].i);
      case 's': return(XbString(f->data[num+index].str));
      case 'n':
      case 'd': return(XbIdentifier(f->data[num+index].str));
      default: return(r);
      }
    }
    return(r);
  }
  Result SetValue::eval(EA3Base *b) const {
    unsigned int index = (unsigned int)expr1.eval(b).toInt();
    Result r = expr2.eval(b);
    if (!b) return(1);
    if (r.type == Result::RS_UNKNOWN) return 0;
    EA3Field *f = b->toField();
    if (f && f->getNum() > index) {
      char dt = f->getDataType(index);
      switch(dt) {
      case 'f': 
        f->data[index].f = r.toFloat(); 
        return(1);
      case 'i': 
        f->data[index].i = (int)r.toInt(); 
        return(1);
      case 'c': 
        f->data[index].i = (int)r.toInt(); 
        return(1);
      case 's': 
        if(r.data.str && (r.type==Result::RS_NAME || r.type == Result::RS_TYPE)) {
          f->data[index].str = r.data.str; 
          return(1);
        } else {
          return(0);
        }
      case 'n':
      case 'd': 
        if(r.data.str && (r.type==Result::RS_NAME || r.type == Result::RS_TYPE)) {
          f->data[index].str = XbIdentifier(r.data.str); 
          return(1);
        } 
      }
    }
    return(0);
  }
  Result _Has::eval(EA3Base *b) const {
    Result r = 0;
    Result r1 = expr1.eval(b);
    XbIdentifier type;
    int matchAll = 0;
    int index = -1;
    if (r1.type == Result::RS_TYPE || r1.type==Result::RS_NAME) {
      type = r1.data.str;
    } else {
      index = (int)r1.toInt();
      matchAll = (index==-1);
    }
    EA3Base *ch;
    int i;
    int num;
    if (b->isBlock()) {  
      EA3Block *bl = b->toBlock();
      if (!bl) return r;
      if (index != -1) {
        ch = bl->getByIndex(index);
        if (!ch) return r;
        return expr2.eval(ch);
      }
      int isArray = bl->isArray();
      if (matchAll || isArray) {
        num = bl->getNumMembers();
		EA3Iterator it = bl->begin();
        for(i=0;it!=it.end()&&i<num;++it,++i) {
          ch = *it;
          if (!matchAll && ch->getType()!=type) continue;
          r = expr2.eval(ch);
          if (r.toInt()) return r;
        }
      } else {
        ch = bl->getByType(type);
        if (ch) {
          return expr2.eval(ch);
        }
      }
    } else if (b->isReference()) {
      EA3Reference *ref = b->toReference();
      if (index != -1) {
        ch = ref->getReference(index);
        if (!ch) return r;
        return expr2.eval(ch);
      }
      num = ref->getNumReferences();
	  EA3Iterator it = ref->begin();
      for(i=0;i<num && it!=it.end();i++,++it) {
        ch = *it;
        if (!matchAll && ch->getType()!=type) continue;
        r = expr2.eval(ch);
        if (r.toInt()) return r;
      }
    }
    return 0;
  }
  Result Format::eval(EA3Base *b) const {
    Result r1 = expr1.eval(b);
    Result r2 = expr2.eval(b);
    
    if (r1.type != Result::RS_TYPE && r1.type !=Result::RS_NAME) {
      return(0);
    }
    char buf[200];
    if (r2.type==Result::RS_TYPE || r2.type==Result::RS_NAME) {
      char *tmp = (char *)EA3Malloc(200+strlen(r2.data.str));
      sprintf(tmp,r1.data.str,r2.data.str);
      Result r = tmp;
      EA3Free(tmp);
      return(r);
    }
    if (r2.type==Result::RS_FLOAT) {
      sprintf(buf,r1.data.str,r2.data.f);
    }
    if (r2.type==Result::RS_INT) {
      sprintf(buf,r1.data.str,r2.data.i);
    }
    if (r2.type==Result::RS_UINT) {
      sprintf(buf,r1.data.str,r2.data.ui);
    }
    if (r2.type == Result::RS_BASE || r2.type == Result::RS_RM_PTR) {
      sprintf(buf,r1.data.str,r2.data.v);
    }
    if (r2.type==Result::RS_VEC3) {
      sprintf(buf,r1.data.str,r2.data.v3()[0],r2.data.v3()[1],r2.data.v3()[2]);
    }
    if (r2.type==Result::RS_MATRIX) {  
      sprintf(buf,r1.data.str,r2.data.m44()[0][0],r2.data.m44()[0][1],r2.data.m44()[0][2],r2.data.m44()[0][3]);
      sprintf(buf,r1.data.str,r2.data.m44()[1][0],r2.data.m44()[1][1],r2.data.m44()[1][2],r2.data.m44()[1][3]);
      sprintf(buf,r1.data.str,r2.data.m44()[2][0],r2.data.m44()[2][1],r2.data.m44()[2][2],r2.data.m44()[2][3]);
      sprintf(buf,r1.data.str,r2.data.m44()[3][0],r2.data.m44()[3][1],r2.data.m44()[3][2],r2.data.m44()[3][3]);
    }
    
    return buf;
  }
  Result SetExtraValue::eval(EA3Base *b) const {
    unsigned int index = (unsigned int)expr1.eval(b).toInt();
    Result r = expr2.eval(b);
    if (!b) return(1);
    EA3Field *f = b->toField();
    if (!f) return(r);
    int num = f->getNum();
    if (f->getNumExtraTypes() > index) {
      const char *et = f->getExtraTypes();
      char dt = et[index];
      switch(dt) {
      case 'f': f->data[num+index].f = r.data.f; break;
      case 'i': f->data[num+index].i = (int)r.data.i; break;
      case 'c': f->data[num+index].i = (int)r.data.i; break;
      case 's': f->data[num+index].str = const_cast<char *>(XbString(r.data.str).getString()); break;
      case 'n':
      case 'd': f->data[num+index].str = const_cast<char *>(XbString(r.data.str).getString()); break;
      }
      return(1);
    }
    return(0);
  }
  Result SetReference::eval(EA3Base *b) const {
    if (!b || !b->isReference()) return(0);
    EA3Base *r = NULL;
    Result r1 = expr1.eval(b);
    Result r2 = expr2.eval(b);
    int ix = (int)r1.toInt();
    if (r2.type == Result::RS_BASE) {
      r = (EA3Base *)r2.data.v;
    } else {
      return 0;
    }
    if (r) {
      b->toReference()->setReference(ix,r);
      return(1);
    }        
    return(0);
  }

  Result InsertReference::eval(EA3Base *b) const {
    if (!b || !b->isReference()) return(0);
    EA3Base *r = NULL;
    Result r1 = expr1.eval(b);
    Result r2 = expr2.eval(b);
    int ix = (int)r1.toInt();
    if (r2.type == Result::RS_BASE) {
      r = (EA3Base *)r2.data.v;
    } else {
      return 0;
    }
    if (r) {
      b->toReference()->insertReference(ix,r);
      return(1);
    }        
    return(0);
  }

  Result _AddReference::eval(EA3Base *b) const {
	  if (!b || !b->isReference()) return(0);
	  EA3Reference* r= b->toReference();
	  if (!r) return 0;
	  Result res=expr.eval(b);
	  if (res.type==Result::RS_BASE)
	  {
		  r->addReference((EA3Base*)res.data.v);
		  return (1);
	  }
	  return (0);
  }

  Result _RemoveReference::eval(EA3Base *b) const {
    //Either an int or an EA3Base
    if (!b||!b->isReference()) return (0);
    EA3Reference* r= b->toReference();
    if (!r) return 0;
    Result res=expr.eval(b);
    if (res.type==Result::RS_BASE)
    {
      return(r->removeReference((EA3Base*)res.data.v));
    }
    return (r->removeReference((int)res.toInt()));

  }

  Result CreateField::eval(EA3Base *b) const {
    Result r = expr1.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      EA3Block *con = b->toBlock();
      if (!con) return(0);
      EA3Base *b2 = con->createField(r.data.str);
      if (b2) {
        expr2.eval(b2);
        return(b2);
      }
    }
    return 0;
  }

  Result CreateBlock::eval(EA3Base *b) const {
    Result r = expr1.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      EA3Block *con = b->toBlock();
      if (!con) return(0);
      EA3Base *b2 = con->createBlock(r.data.str);
      if (b2) {
        expr2.eval(b2);
        return(b2);
      }
    }
    return 0;
  }

  Result CreateArray::eval(EA3Base *b) const {
    Result r = expr1.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      EA3Block *con = b->toBlock();
      if (!con) return(0);
      EA3Base *b2 = con->createArray(r.data.str);
      if (b2) {
        expr2.eval(b2);
        return(b2);
      }
    }
    return 0;
  }

  Result CreateReference::eval(EA3Base *b) const {
    Result r = expr1.eval(b);
    if (r.type == Result::RS_NAME || r.type == Result::RS_TYPE) {
      EA3Block *con = b->toBlock();
      if (!con) return(0);
      EA3Base *b2 = con->createReference(r.data.str);
      if (b2) {
        expr2.eval(b2);
        return(b2);
      }
    }
    return 0;
  }

  Result GetContainer::eval(EA3Base *b) const {
    b = b->getContainer();
    while (b && !expr1.eval(b)) {
      b = b->getContainer();
    }
    if (!b) return(0);
    return (expr2.eval(b));
  }

	Result _MapSize::eval(EA3Base *b) const {
		Result rmap = expr.eval(b);
		if (rmap.type != Result::RS_RM_PTR) return Result();
		ResultsMap *rm = static_cast<ResultsMap*>(rmap.data.v);
		return rm->getSize();
	}

	Result MapGetValue::eval(EA3Base *b) const {
		Result rmap = expr1.eval(b);
		if (rmap.type != Result::RS_RM_PTR) return Result();
		ResultsMap *rm = static_cast<ResultsMap*>(rmap.data.v);
		return (*rm)[expr2.eval(b)].eval(b);
	}

	Result MapSetValue::eval(EA3Base *b) const {
		Result rmap = expr1.eval(b);
		if (rmap.type != Result::RS_RM_PTR) return Result();
		ResultsMap *rm = static_cast<ResultsMap*>(rmap.data.v);
		Result r = expr3.eval(b);
		((*rm)[expr2.eval(b)] = r).eval(b);
		return r;
	}
	Result _MapClear::eval(EA3Base *b) const {
		Result rmap = expr.eval(b);
		if (rmap.type != Result::RS_RM_PTR) return Result();
		ResultsMap *rm = static_cast<ResultsMap*>(rmap.data.v);
		rm->clear();
		return 1;
	}
	

  EA3_EXPORT void find(EA3Base *root, XbPArray<EA3Base *> &outlist,const Expression &e) {
    if (e.eval(root).toInt()) {
      outlist.append(root);
    }
    EA3Block *bl = root->toBlock();
    if (bl) {
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
      for(int i = 0; it != it.end() && i < num; ++it,i++) {
        find(*it,outlist,e);
      }
    }
  }
  EA3_EXPORT void find(EA3Base *root, XbPArray<EA3Base *> &outlist,const Expression &e, const Expression &recurseWhen) {
    if (e.eval(root).toInt()) {
      outlist.append(root);
    }
    EA3Block *bl = root->toBlock();
    if (bl) {
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
      for(int i = 0; it != it.end() && i < num; ++it,i++) {
        EA3Base *b = *it;
        if (!recurseWhen.eval(b).toInt()) continue;
        find(b,outlist,e,recurseWhen);
      }
    }
  }
  EA3_EXPORT void find(XbPArray<EA3Base *> &inList, XbPArray<EA3Base *> &outList,const Expression &e) {
    for(int i=0;i<inList.getSize();i++) {
      if (e.eval(inList[i]).toInt()) {
        outList.append(inList[i]);
      }
    }
  }
  EA3_EXPORT EA3Pointer findFirst(EA3Base *root,const Expression &e) {
    if (e.eval(root).toInt()) {
      return(root);
    }
    EA3Block *bl = root->toBlock();
    if (bl) {
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
      for(int i = 0; it != it.end() && i < num; ++it,i++) {
        EA3Base *res = findFirst(*it,e);
        if(res) return(res);
      }
    }
    return(NULL);
  }
  EA3_EXPORT EA3Pointer findFirst(EA3Base *root,const Expression &e, const Expression &recurseWhen) {
    if (e.eval(root).toInt()) {
      return(root);
    }
    EA3Block *bl = root->toBlock();
    if (bl) {
      int num = bl->getNumMembers();
	  EA3Iterator it = bl->begin();
      for(int i = 0; it != it.end() && i < num; ++it,i++) {
        EA3Base *b = *it;
        if (!recurseWhen.eval(b).toInt()) continue;
        EA3Base *res = findFirst(b,e,recurseWhen);
        if(res) return(res);
      }
    }
    return(NULL);
  }
  EA3_EXPORT EA3Pointer findFirst(XbPArray<EA3Base *> &inList, const Expression &e) {
    for(int i=0;i<inList.getSize();i++) {
      if (e.eval(inList[i]).toInt()) {
        return(inList[i]);
      }
    }
    return(NULL);
  }
  
#define EXPR_EVALUATOR(op,res) EA3_EXPORT Result Expression::eval_ ## op(const Expression *e,EA3Base *b) {res}
  EXPR_EVALUATOR(OP_AND,return(e->lhs->eval(b).toInt() && e->rhs->eval(b).toInt());)
  EXPR_EVALUATOR(OP_OR, return(e->lhs->eval(b).toInt() || e->rhs->eval(b).toInt());)
  EXPR_EVALUATOR(OP_EQUAL, return(e->lhs->eval(b) == e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_NEQUAL, return(e->lhs->eval(b) != e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_PLUS, return(e->lhs->eval(b) + e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_MINUS, return(e->lhs->eval(b) - e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_MULT, return(e->lhs->eval(b) * e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_DIV, return(e->lhs->eval(b) / e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_GT, return(e->lhs->eval(b) > e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_LT, return(e->lhs->eval(b) < e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_LTE, return(e->lhs->eval(b) <= e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_GTE, return(e->lhs->eval(b) >= e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_BW_AND, return(e->lhs->eval(b) & e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_BW_OR, return(e->lhs->eval(b) | e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_MOD, return(e->lhs->eval(b) % e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_SHIFT_LEFT, return(e->lhs->eval(b) << e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_SHIFT_RIGHT, return(e->lhs->eval(b) >> e->rhs->eval(b));)
  EXPR_EVALUATOR(OP_JOIN, {
    e->lhs->eval(b);
    return(e->rhs->eval(b));
  })
  EXPR_EVALUATOR(OP_NOT, return(!e->lhs->eval(b));)
  EXPR_EVALUATOR(OP_NEG, return(-e->lhs->eval(b));)
    
  Expression& Expression::operator=(const Expression &other) {
    delete lhs;
    delete rhs;
    delete query;
    lhs = other.lhs? new Expression(*other.lhs) : NULL;
    rhs = other.rhs? new Expression(*other.rhs) : NULL;
    query = other.query? other.query->copy() : NULL;
    evaluatorCB = other.evaluatorCB;
    val = other.val;
    return(*this);
  }
  Expression::~Expression() {
    delete lhs;
    delete rhs;
    delete query;
  }

}

