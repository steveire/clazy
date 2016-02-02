#include <QtCore/qglobal.h>

struct A
{
    int v;
};

extern void foo(int);
extern void foo2(A*);

void test1()
{
    A a;
    a.v = 1; // Warn
    int noise;
    noise++;
    noise = 1;
}

void test2()
{
    A a;
    a.v = 1; // Warn
}

void test3()
{
    A a;
    a.v = 1; // OK
    foo(a.v);
    foo2(&a);
}

class Test4
{
public:

    Test4() : m_a(A())
    {
        m_a.v = 1;
        foo2(&m_a);
    }

    void method()
    {
        m_a.v = 1; // OK
    }

    A method2()
    {
        A a;
        a.v = 1;
        return a;
    }

    A m_a;
};

void test5()
{
    A a;
    while (true)
        a.v = 1; // OK
}

void test6(A &a)
{
    a.v = 1; // OK
}

struct RAIIClass
{
    RAIIClass() {}
    ~RAIIClass() {}
    int v;
};

struct RAIIClass2
{
    RAIIClass r;
    int v;
};

void test7()
{
    RAIIClass a;
    a.v = 1; // OK
}

void test8()
{
    RAIIClass2 r;
    r.v = 1; // OK
}

int* test9()
{
    A a;
    return &a.v;
}

Q_GLOBAL_STATIC(A, s_a10)

A test11()
{
    A a, b;
    a.v = 1;
    return a;
}

static A s_a12;
void test12()
{
    s_a12.v = 1; // OK
}

void test13()
{
    A a;
    A *a2 = &a;
    a.v = 1; // OK
}

void test14()
{
    A a;
    int *v = &(a.v);
    a.v = 1; // OK
}
