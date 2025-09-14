#ifndef growable_array_h_INCLUDED
#define growable_array_h_INCLUDED

template <class T>
class GrowableArray
{
private:
    typedef enum { StartArraySize = 128 } bogus;
    size_t num_elem;
    size_t array_size;
    T *pArray;

private:

    bool twice()
    {
        bool rslt;

        T *old_array = pArray;
        size_t new_size = array_size * 2;

        pArray = new T [ new_size ];
        if (pArray != 0)
        {
            rslt = true;
            unsigned int i;
            for (i = 0; i < array_size; i++)
            {
                pArray[i] = old_array[i];
            }

            delete [] old_array;

            for (i = 0; i < array_size; i++)
            {
                pArray[array_size+i] = 0;
            }

            array_size = new_size;
        }
        else
        {
            rslt = false;
        }

        return rslt;
    }


public:
    GrowableArray()
    {
        pArray = new T[ StartArraySize ];
        for (unsigned int i = 0; i < StartArraySize; i++)
        {
            pArray[i] = 0;
        }
        num_elem = 0;
        array_size = StartArraySize;
    }

    ~GrowableArray()
    {
        if (pArray != NULL) {
            delete [] pArray;
        }
    }

    const size_t length(void) const { return num_elem; }
    void set_to_zero() { num_elem = 0; }
    T &operator[](const size_t i)
    {
        assert( i < num_elem );
        return pArray[ i ];
    }
    T operator[](const size_t i ) const
    {
        assert( i < num_elem );
        return pArray[ i ];
    }
    const T *getData() const { return pArray; }
    void append( T item )
    {
        if (num_elem == array_size)
        {
            bool allocOK = twice();
            assert( allocOK );
        }

        pArray[ num_elem ] = item;
        num_elem++;
    }

    void expand( size_t amount )
    {
        bool allocOK = true;

        while (allocOK && num_elem + amount >= array_size)
        {
            allocOK = twice();
            assert( allocOK );
        }
        num_elem += amount;
    }

    void ensure( size_t target )
    {
        int total = target - num_elem + 1;
        if (total > 0)
        {
            expand(total);
        }
    }

    void remove(void)
    {
        if (num_elem > 0)
            num_elem--;
    }

};

#endif

