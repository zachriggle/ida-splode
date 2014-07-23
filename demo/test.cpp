#include <stdio.h>
#include <malloc.h>
#include <Windows.h>
#include <intrin.h>

//
// Sample structure to test structure reconstruction,
// heap-allocation offsets, and type x-refs.
//
#include <pshpack1.h>
struct S {
    char  a;
    short b;
    int   c;
    S() { c = 0x1230;
	      b = 0x1231;
		  a = 1; }
    void setA() { a = 1; }
    void setB() { b = 0x1232; }
    void setC() { c = 0x1233; }
};
#include <poppack.h>

void UseS(S* s) { s->c = 0x1234; }

void TestStruct() {
    S *s = new S();
    s->setA();
    s->setB();
    s->setC();

    UseS(s);

    delete s;
}

//
// Sample structure to demonstrate vtable resolution.
//
struct V {
    virtual ~V(){};
    virtual void Foo()=0;
};
struct V1 : public V { void Foo(){} };
void TestVtable() {
    V *v = new V1;
    v->Foo();
}

//
// Sample structure to demonstrate pointer members,
// and heap-pointers-to-heap-pointers.
//
struct A { A() { i=0x1200; } int i; };
struct B {
    A* a;
    B()  { a = new A; }
    ~B() { delete a; }
};
void TestPointerMembers() {
    B* b = new B();
    b->a->i = 0x1235;
    delete b;
}

//
// Check allocator PageHeap.
//
void TestMalloc() {
    void* v = malloc(0x1234);
}

//
// Simulate a custom allocator.
//
// By adding a PageHeap redirector, or info on a custom
// allocator, we can track these heap allocations.
//
int heap[0x100];
void* custom_malloc(int size) {
    fprintf(stderr, "Should not hit this code\n");
    for(size_t i = 0; i < ARRAYSIZE(heap); i++)
        heap[i]=i;
    return &heap[0x80];
}

void TestCustomMalloc() {
    void** memory = (void**) custom_malloc(0x1236);


    // Look for PageHeap magic value before the allocation
    __try
    {int magic = *((int*) memory - 1);}
    __except(1){}
}

//
// Example to show a stack-based variable being passed by
// pointer down a few call frames.
//
void stack_var_user(int* C) {
    *C = 0x1237;
}
void stack_var_middleman(int* B) {
    stack_var_user(B);
}
void TestStackVars() {
    int A = 0x1238;
    stack_var_middleman(&A);
}
#include <iso646.h>
//
// Example to show function pointer resolution
//
typedef void (WINAPI * PfnODS)(char*);
PfnODS pODS = NULL;
#define szMod     "kernel32.dll"
#define szFnName  "OutputDebugStringA"
void TestDynamicIAT() {
    HMODULE h = GetModuleHandleA(szMod);
    pODS = (PfnODS) GetProcAddress(h, szFnName);
    pODS("0x1239");
}

//
// Example to show MEM_MOVEABLE HGLOBAL resolution
//
void TestMoveableHGLOBAL() {
    HGLOBAL g = GlobalAlloc(GHND, 0x123a);
    void *  m = GlobalLock(g);
    GlobalUnlock(m);
    GlobalFree(g);
}

//
// Example to show branch statistics
//
void TestBranchStatistics() {
    int x = 0, ten=10, eight=8;
    for(int i = 0; i < ten; i++)
    {
        if(i >= eight) { x++; }
        else           { x--; }
    }
}

//
// Test exceptions and invalid memory access.
//
// Pintool will attempt to resolve the type of memory
// at all of these addresses and should get nothing.
//
void TestInvalidMemoryAccess() {
    DWORD LooksLikeMemory = 0x0eadbeef;
    volatile DWORD* Zero   = (DWORD*) 0;
    __try
    {
        *Zero=0x123b;
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        LooksLikeMemory = 0x000df00d;
    }
}


//
// Chew up time without touching (much) memory.
//
// 'pragma optimize' and the bit-and operation are both necessary
// for the compiler to loop/increment/compare in a register,
// and save the result into the global variable at the end.
//
#pragma optimize("gty", on)
int g_i;
void TestTimeExpense() {
    for(g_i = 0; (g_i & 0xffff) < 0xffff; g_i++);
}
#pragma optimize("", on)



int main() {
    fprintf(stderr, "Begin test\n");
    TestStruct();
    TestVtable();
    TestBranchStatistics();
    TestMoveableHGLOBAL();
    TestDynamicIAT();
    TestStackVars();
    TestMalloc();
    TestCustomMalloc();
    TestInvalidMemoryAccess();
    TestPointerMembers();
    TestTimeExpense();
    fprintf(stderr, "End test\n");
    return 1;
}
