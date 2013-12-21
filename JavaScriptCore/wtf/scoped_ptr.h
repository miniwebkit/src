
#ifndef __base_scoped_ptr_h__
#define __base_scoped_ptr_h__

#pragma once

#include <assert.h>
#include <cstddef>
#include <stdlib.h>

// Scopers能帮助你管理指针的所有权, 在作用域结束时自动销毁维护的指针.
// 有两个类分别对应new/delete和new[]/delete[]操作.
//
// 用法示例(scoped_ptr):
//     {
//       scoped_ptr<Foo> foo(new Foo("wee"));
//     } // foo goes out of scope, releasing the pointer with it.
//
//     {
//       scoped_ptr<Foo> foo;          // No pointer managed.
//       foo.reset(new Foo("wee"));    // Now a pointer is managed.
//       foo.reset(new Foo("wee2"));   // Foo("wee") was destroyed.
//       foo.reset(new Foo("wee3"));   // Foo("wee2") was destroyed.
//       foo->Method();                // Foo::Method() called.
//       foo.get()->Method();          // Foo::Method() called.
//       SomeFunc(foo.release());      // SomeFunc takes ownership, foo no longer
//                                     // manages a pointer.
//       foo.reset(new Foo("wee4"));   // foo manages a pointer again.
//       foo.reset();                  // Foo("wee4") destroyed, foo no longer
//                                     // manages a pointer.
//     } // foo wasn't managing a pointer, so nothing was destroyed.
//
// 用法示例(scoped_array):
//     {
//       scoped_array<Foo> foo(new Foo[100]);
//       foo.get()->Method();  // Foo::Method on the 0th element.
//       foo[10].Method();     // Foo::Method on the 10th element.
//     }


// scoped_ptr<T>和T*很像, 只是会在析构的时候自动销毁维护的指针.
// 也就是说, scoped_ptr<T>拥有T对象的所有权.
// 和T*一样, scoped_ptr<T>内部的指针可能为NULL或者指向T对象, 且线程安全,
// 一旦解引用, 线程安全性就依赖于T本身.
//
// scoped_ptr对象很小: sizeof(scoped_ptr<C>) == sizeof(C*).
template<class C>
class scoped_ptr
{
public:
    // 元素类型.
    typedef C element_type;

    // 构造函数. 缺省用NULL初始化, 不可能创建一个未初始化的scoped_ptr.
    // 输入参数必须使用new分配.
    explicit scoped_ptr(C* p=NULL) : ptr_(p) {}

    // 析构函数, 删除内部的对象. 无须判断ptr_==NULL, C++自己会处理.
    ~scoped_ptr()
    {
        enum { type_must_be_complete = sizeof(C) };
        delete ptr_;
    }

    // 重置. 删除现有的对象, 接管新的对象所有权.
    void reset(C* p=NULL)
    {
        if(p != ptr_)
        {
            enum { type_must_be_complete = sizeof(C) };
            delete ptr_;
            ptr_ = p;
        }
    }

    // 访问拥有的对象.
    // operator*和operator->在没有对象的时候会产生断言.
    C& operator*() const
    {
        assert(ptr_ != NULL);
        return *ptr_;
    }
    C* operator->() const
    {
        assert(ptr_ != NULL);
        return ptr_;
    }
    C* get() const { return ptr_; }

    // 比较操作.
    // 判断scoped_ptr引用的对象是否为p, 而不仅仅是两对象相等.
    bool operator==(C* p) const { return ptr_ == p; }
    bool operator!=(C* p) const { return ptr_ != p; }

    // 互换指针.
    void swap(scoped_ptr& p2)
    {
        C* tmp = ptr_;
        ptr_ = p2.ptr_;
        p2.ptr_ = tmp;
    }

    // 释放指针.
    // 返回当前拥有的对象指针, 如果为空则返回NULL.
    // 操作完成之后拥有空指针, 不再拥有任何对象.
    C* release()
    {
        C* retVal = ptr_;
        ptr_ = NULL;
        return retVal;
    }

private:
    C* ptr_;

    // 禁止和scoped_ptr类型比较. C2!=C的比较没有任何意义, C2==C也没什么意义因为
    // 你不可能让两个对象拥有相同的指针.
    template<class C2> bool operator==(scoped_ptr<C2> const& p2) const;
    template<class C2> bool operator!=(scoped_ptr<C2> const& p2) const;

    scoped_ptr(const scoped_ptr&);
    void operator=(const scoped_ptr&);
};

template<class C>
void swap(scoped_ptr<C>& p1, scoped_ptr<C>& p2)
{
    p1.swap(p2);
}

template<class C>
bool operator==(C* p1, const scoped_ptr<C>& p2)
{
    return p1 == p2.get();
}

template<class C>
bool operator!=(C* p1, const scoped_ptr<C>& p2)
{
    return p1 != p2.get();
}

// scoped_array<C>和scoped_ptr<C>类似, 但调用者必须使用new[]分配对象, 类析
// 构的时候会用delete[]删除对象.
//
// scoped_array<C>要么指向一个对象要么指向NULL, 拥有指针的所有权, 线程安全.
// 一旦索引指针, 返回对象的线程安全性依赖T本身.
//
// 大小: sizeof(scoped_array<C>) == sizeof(C*).
template<class C>
class scoped_array
{
public:
    // 元素类型.
    typedef C element_type;

    // 构造函数. 缺省用NULL初始化, 不可能创建一个未初始化的scoped_array.
    // 输入参数必须使用new[]分配.
    explicit scoped_array(C* p = NULL) : array_(p) { }

    // 析构函数, 删除内部的对象. 无须判断ptr_==NULL, C++自己会处理.
    ~scoped_array()
    {
        enum { type_must_be_complete = sizeof(C) };
        delete[] array_;
    }

    // 重置. 删除现有的对象, 接管新的对象所有权.
    // this->reset(this->get())能正常工作.
    void reset(C* p = NULL)
    {
        if(p != array_)
        {
            enum { type_must_be_complete = sizeof(C) };
            delete[] array_;
            array_ = p;
        }
    }

    // 获取指针指向的索引对象.
    // 如果指针为空或者索引为负数会产生断言.
    C& operator[](std::ptrdiff_t i) const
    {
        assert(i >= 0);
        assert(array_ != NULL);
        return array_[i];
    }

    // 获取指针对象.
    C* get() const
    {
        return array_;
    }

    // 比较操作.
    // 判断scoped_array引用的对象是否为p, 而不仅仅是两对象相等.
    bool operator==(C* p) const { return array_ == p; }
    bool operator!=(C* p) const { return array_ != p; }

    // 互换指针.
    void swap(scoped_array& p2)
    {
        C* tmp = array_;
        array_ = p2.array_;
        p2.array_ = tmp;
    }

    // 释放数组指针.
    // 返回当前拥有的对象指针, 如果为空则返回NULL.
    // 操作完成之后拥有空指针, 不再拥有任何对象.
    C* release()
    {
        C* retVal = array_;
        array_ = NULL;
        return retVal;
    }

private:
    C* array_;

    // 禁止和scoped_array类型比较.
    template<class C2> bool operator==(scoped_array<C2> const& p2) const;
    template<class C2> bool operator!=(scoped_array<C2> const& p2) const;

    scoped_array(const scoped_array&);
    void operator=(const scoped_array&);
};

template<class C>
void swap(scoped_array<C>& p1, scoped_array<C>& p2)
{
    p1.swap(p2);
}

template<class C>
bool operator==(C* p1, const scoped_array<C>& p2)
{
    return p1 == p2.get();
}

template<class C>
bool operator!=(C* p1, const scoped_array<C>& p2)
{
    return p1 != p2.get();
}

// 类封装C函数库的free(), 用作scoped_ptr_malloc的模板参数
class ScopedPtrMallocFree
{
public:
    inline void operator()(void* x) const
    {
        free(x);
    }
};

// scoped_ptr_malloc<>类似scoped_ptr<>, 接受第二个模板参数用于释放对象.
template<class C, class FreeProc=ScopedPtrMallocFree>
class scoped_ptr_malloc
{
public:
    // 元素类型.
    typedef C element_type;

    // 构造函数. 缺省用NULL初始化, 不可能创建一个未初始化的scoped_ptr_malloc.
    // 输入对象的分配器必须和释放函数的匹配. 对于缺省的释放函数, 分配函数可以
    // 是malloc、calloc或realloc.
    explicit scoped_ptr_malloc(C* p=NULL): ptr_(p) {}

    // 析构函数. 如果是C对象, 调用free函数.
    ~scoped_ptr_malloc()
    {
        free_(ptr_);
    }

    // 重置. 删除现有的对象, 接管新的对象所有权.
    // this->reset(this->get())能正常工作.
    void reset(C* p=NULL)
    {
        if(ptr_ != p)
        {
            free_(ptr_);
            ptr_ = p;
        }
    }

    // 访问拥有的对象.
    // operator*和operator->在没有对象的时候会产生断言.
    C& operator*() const
    {
        assert(ptr_ != NULL);
        return *ptr_;
    }

    C* operator->() const
    {
        assert(ptr_ != NULL);
        return ptr_;
    }

    C* get() const
    {
        return ptr_;
    }

    // 比较操作.
    // 判断scoped_ptr_malloc引用的对象是否为p, 而不仅仅是两对象相等.
    // 为了保持和boost实现的兼容, 参数是非const类型.
    bool operator==(C* p) const
    {
        return ptr_ == p;
    }

    bool operator!=(C* p) const
    {
        return ptr_ != p;
    }

    // 互换指针.
    void swap(scoped_ptr_malloc & b)
    {
        C* tmp = b.ptr_;
        b.ptr_ = ptr_;
        ptr_ = tmp;
    }

    // 释放指针.
    // 返回当前拥有的对象指针, 如果为空则返回NULL.
    // 操作完成之后拥有空指针, 不再拥有任何对象.
    C* release()
    {
        C* tmp = ptr_;
        ptr_ = NULL;
        return tmp;
    }

private:
    C* ptr_;

    template<class C2, class GP>
    bool operator==(scoped_ptr_malloc<C2, GP> const& p) const;
    template<class C2, class GP>
    bool operator!=(scoped_ptr_malloc<C2, GP> const& p) const;

    static FreeProc const free_;

    scoped_ptr_malloc(const scoped_ptr_malloc&);
    void operator=(const scoped_ptr_malloc&);
};

template<class C, class FP>
FP const scoped_ptr_malloc<C, FP>::free_ = FP();

template<class C, class FP> inline
void swap(scoped_ptr_malloc<C, FP>& a, scoped_ptr_malloc<C, FP>& b)
{
    a.swap(b);
}

template<class C, class FP> inline
bool operator==(C* p, const scoped_ptr_malloc<C, FP>& b)
{
    return p == b.get();
}

template<class C, class FP> inline
bool operator!=(C* p, const scoped_ptr_malloc<C, FP>& b)
{
    return p != b.get();
}

#endif //__base_scoped_ptr_h__