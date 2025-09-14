// (c) 1998-2010 Electronic Arts Inc.

#ifndef EA3Mem_h
#define EA3Mem_h
//#include <malloc.h>
#include <eabase/eabase.h>

/*
  template<class _Tp>
  class my_alloc {

  public:
  static _Tp* allocate(size_t __n)
  { return 0 == __n ? 0 : (_Tp*) EA3Malloc(__n); }
  static _Tp* allocate(void)
  { return (_Tp*) EA3Malloc(sizeof (_Tp)); }
  static void deallocate(_Tp* __p, size_t)
  { if (0 != __n) EA3Free(__p); }
  static void deallocate(_Tp* __p)
  { EA3Free(__p); }
  }; 
*/


#ifndef EA3_EXPORT
#  if defined(EA_PLATFORM_WINDOWS) && defined(EA_DLL)
#    ifdef EA3_EXPORTS
#      define EA3_EXPORT __declspec(dllexport)
#    else
#      define EA3_EXPORT __declspec(dllimport)
#    endif
#  else
#    define EA3_EXPORT
#  endif
#endif

extern "C"
{
    EA3_EXPORT void  EA3Free    (void *ptr);

    EA3_EXPORT void *EA3Malloc  (size_t size);

    EA3_EXPORT void *EA3Realloc (void *ptr, size_t size);

    EA3_EXPORT void ReportMemoryLeaks(const char* aFilename);
}

class EA3Mem
{
public:
#ifndef SWIG
    inline void *operator new(size_t, void *v)
    {
        return(v);
    }
    inline void *operator new(size_t s)
    {
        return EA3Malloc(s);
    }
    inline void operator delete(void *v)
    {
        EA3Free(v);
    }
    inline void operator delete(void *, void *)
    {
    }
    inline void *operator new[](size_t s)
    {
        return(EA3Malloc(s));
    }
    inline void operator delete[](void *v)
    {
        EA3Free(v);
    }
#endif
};


class StlAllocator
{
public:
    StlAllocator(const char* pName = "")
        : mName(pName)
    {
    }

    StlAllocator(const StlAllocator& x)
        : mName(x.mName)
    {
    }

    StlAllocator(const StlAllocator& x, const char* pName)
        : mName(pName)
    {
    }

    StlAllocator& operator=(const StlAllocator& x)
    {
        mName = x.mName;
        return *this;
    }

    void* allocate(size_t n, int flags = 0)
    {
        return EA3Malloc(n);
    }

    void* allocate(size_t n, size_t alignment, size_t offset, int flags = 0)
    {
        return EA3Malloc(n);
    }

    void deallocate(void* p, size_t n)
    {
        EA3Free(p);
    }

    const char* get_name() const
    {
        return mName;
    }

    void set_name(const char* pName)
    {
        mName = pName;
    }

protected:
	const char* mName;
};

inline bool operator==(const StlAllocator& , const StlAllocator& )
{
    return true; // All StlAllocators are considered equal, as they merely use global new/delete.
}

inline bool operator!=(const StlAllocator& , const StlAllocator& )
{
    return false; // All StlAllocators are considered equal, as they merely use global new/delete.
}


#endif // EA3Mem_h
