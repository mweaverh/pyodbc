
#ifndef _WRAPPER_H_
#define _WRAPPER_H_

class Object
{
protected:
    PyObject* p;

    // GCC freaks out if these are private, but it doesn't use them (?)
    // Object(const Object& illegal);
    // void operator=(const Object& illegal);

public:
    Object(PyObject* _p = 0)
    {
        p = _p;
    }

    ~Object()
    {
        Py_XDECREF(p);
    }

    Object& operator=(PyObject* pNew)
    {
        Py_XDECREF(p);
        p = pNew;
        return *this;
    }

    bool IsValid() const { return p != 0; }

    bool Attach(PyObject* _p)
    {
        // Returns true if the new pointer is non-zero.

        Py_XDECREF(p);
        p = _p;
        return (_p != 0);
    }

    PyObject* Detach()
    {
        PyObject* pT = p;
        p = 0;
        return pT;
    }

    operator PyObject*()
    {
        return p;
    }

    PyObject* Get()
    {
        return p;
    }
};


class Tuple
    : public Object
{
public:

    Tuple(PyObject* _p = 0)
        : Object(_p)
    {
    }

    operator PyTupleObject*()
    {
        return (PyTupleObject*)p;
    }

    PyObject*& operator[](int i)
    {
        I(p != 0);
        return PyTuple_GET_ITEM(p, i);
    }

    Py_ssize_t size() { return p ? PyTuple_GET_SIZE(p) : 0; }
};


#ifdef WINVER
struct RegKey
{
    HKEY hkey;

    RegKey()
    {
        hkey = 0;
    }

    ~RegKey()
    {
        if (hkey != 0)
            RegCloseKey(hkey);
    }

    operator HKEY() { return hkey; }
};
#endif


template<class T> struct Ptr
{
    T* pb;
    size_t cb;
    bool is_owner;
    // If true, free pb.  Will be false if we are pointing at a stack buffer.

    Ptr()
    {
        pb = 0;
        cb = 0;
        is_owner = true;
    }

    Ptr(T* _pb, size_t _cb, bool _is_owner)
    {
        pb = _pb;
        cb = _cb;
        is_owner = _is_owner;
    }

    ~Ptr()
    {
        if (pb && is_owner)
            pyodbc_free(pb);
    }

    bool AllocateByteLength(size_t cbNew)
    {
        // Allocates given a buffer size in bytes.

        T* pbNew = (T*)pyodbc_malloc(cbNew);

        if (!pbNew)
        {
            PyErr_NoMemory();
            return false;
        }

        if (pb && is_owner)
            pyodbc_free(pb);

        pb       = pbNew;
        cb       = cbNew;
        is_owner = true;
        return true;
    }

    bool ReallocByteLength(size_t cbNew)
    {
        // Save the old buffer and allocate.  If successful, copy the old buffer
        // to the new buffer and free the old buffer.  If allocation fails, keep
        // the old buffer.

        T* pbT = pb;
        size_t cbT = cb;

        pb = 0;
        cb = 0;

        if (!AllocateByteLength(cbNew))
        {
            pb = pbT;
            cb = cbT;
            return false;
        }

        if (pbT)
        {
            memcpy(pb, pbT, cb);
            pyodbc_free(pbT);
        }

        return true;
    }

    inline bool AllocateDataLength(size_t c)
    {
        return AllocateByteLength(c * sizeof(T));
    }

    size_t ByteLength()
    {
        return cb;
    }

    size_t DataLength()
    {
        return cb / sizeof(T);
    }

    operator T*() { return pb; }
};


#endif // _WRAPPER_H_
