#pragma once
/*
 * instrumenter.h
 *
 *  Created on: Dec 18, 2013
 */

#include "common.h"


struct Counter
{
    UINT64  a;
    UINT64  * t;
    Counter
    (
        UINT64 * t_
    ) : t(t_)
    {
        a = GetTickCount64();
    }



    ~Counter
    (
    )
    {
        *t += GetTickCount64() - a;
    }



};


template<typename T>
string
NameOfType
(
);


template<>
inline string NameOfType<INS
                         >
(
)
{
    return "INS";
}



template<>
inline string NameOfType<IMG
                         >
(
)
{
    return "IMG";
}



template<>
inline string NameOfType<TRACE
                         >
(
)
{
    return "TRACE";
}



/**
 * Provides weak ordering to PIN's instrumentation, and an instance-based
 * mechanism to get instrumentation callbacks for various types.
 *
 * Simply provides a static callback method #InstrumentationFunction which
 * should be passed to the appropriate *_AddInstrumentationFunction before
 * starting the target binary.
 *
 * Example use cases:
 *      Need to load symbols before searching for symbols within loaded module.
 *      Need to use PIN_ExecuteAt for a specific instruction before general
 *      instrumentation does anything else.
 */
template<typename T>
class Instrumenter : public virtual SortableMixin
{
protected:
    UINT64  NumberInstrumented;
    UINT64  TimeInstrumenting;

    /**
     * Set of all instances created, sorted by the order provided
     * by #SortableMixin.
     */
    static multiset<Instrumenter<T> *, SortableMixin::PtrComparator> * Instances;

    /**
     * Registers the instance in #Instances
     */
    Instrumenter
    (
    );

    /** Given an instance of T, retrieve its address */
    ADDRINT
    GetAddress
    (
        T t
    );


    /**
     * Unregisters the instance in #Instances
     */
    virtual
    ~Instrumenter
    (
    );

    /**
     * Instrumentation function to be overridden by the implementing class.
     * @param t
     */
    virtual void
    Instrument
    (
        T t
    ) = 0;


    /**
     * Filter function to be overridden by the implementing class.
     *
     * While this logic could be encapsulated in #Instrument, by separating
     * it into a different virtual routine, we can have useful mixins.
     *
     * Default behavior is to only return 'true' if the address for \c t
     * is in a whitelisted module.
     */
    virtual bool
    ShouldInstrument
    (
        T t
    );


    static void
    OnFini
    (
        INT32,
        void * self_
    )
    {
        Instrumenter<T> *self = (Instrumenter<T> *)self_;

//        if (self)
//        {
//            cerr    << "[pintool] " << typeid(*self).name() << " used " << decstr(self->TimeInstrumenting)
//                    << " ms adding instrumentation" << endl;
//        }
//        else
//        {
//            cerr    << "[pintool] " << NameOfType<T>() << " used " << decstr(TotalTimeInstrumenting)
//                    << " ms adding instrumentation" << endl;
//        }
    }



public:
    static UINT64   Counter;
    static UINT64   TotalTimeInstrumenting;
    /**
     * Calls #Instrument on all instances in #Instances in the order provided
     * by #SortableMixin.
     */
    static void
    InstrumentationFunction
    (
        T       t,
        void    *
    );


};

/** Specialization of #Instrumenter to instrument instructions */
#define InstructionInstrumenter Instrumenter < INS >

/** Specialization of #Instrumenter to instrument images */
#define ImageInstrumenter       Instrumenter < IMG >

/** Specialization of #Instrumenter to instrument traces */
#define TraceInstrumenter       Instrumenter < TRACE >

//******************************************************************************
//                    INSTRUMENTER TEMPLATE IMPLEMENTATIONS
//******************************************************************************
template<typename T>
multiset<Instrumenter<T> *, SortableMixin::PtrComparator> * Instrumenter<T>::Instances = NULL;
template<typename T>
UINT64 Instrumenter<T>::TotalTimeInstrumenting  = 0;
template<typename T>
UINT64 Instrumenter<T>::Counter                 = 0;

//------------------------ INSTRUMENTER :: CONSTRUCTOR -------------------------
template<typename T>
Instrumenter<T>::Instrumenter
(
)
{
    if (NULL == Instances)
    {
        Instances = new multiset<Instrumenter<T> *, SortableMixin::PtrComparator>;
        PIN_AddFiniFunction(OnFini, NULL);
    }

    if (NULL != Instances)
    {
        Instances->insert(this);
    }

    NumberInstrumented  = 0;
    TimeInstrumenting   = 0;
    PIN_AddFiniFunction(OnFini, this);
}



//------------------------- INSTRUMENTER :: DESTRUCTOR -------------------------
template<typename T>
Instrumenter<T>::~Instrumenter
(
)
{
    if (NULL != Instances)
    {
        Instances->erase(this);
    }

    // There is no way to 'un-instrument' anything in PIN.
    // Instrumentation must be fully reset, which will cause all of the
    // instrumentation to be called to reinstrument code.
    PIN_RemoveInstrumentation();
}



//--------------------- INSTRUMENTER :: SHOULD INSTRUMENT ----------------------
template<typename T>
inline bool
Instrumenter<T >::ShouldInstrument
(
    T t
)
{

    return IsAddressWhitelisted(GetAddress(t));
}



//------------- INSTRUMENTER :: INSTRUMENTATION FUNCTION DELEGATOR -------------
template <typename T>
void
Instrumenter<T >::InstrumentationFunction
(
    T       t,
    void    *
)
{
    UINT64 start = GetTickCount64();

    if (NULL != Instances)
    {
        for (auto iter = Instances->begin();
             iter != Instances->end();
             iter++)
        {
            Instrumenter<T> * inst  = *iter;

            UINT64 a                = GetTickCount64();

            if (inst->ShouldInstrument(t))
            {
                inst->NumberInstrumented++;
                inst->Instrument(t);
            }

            inst->TimeInstrumenting += (GetTickCount64() - a);
        }
    }

    TotalTimeInstrumenting += (GetTickCount64() - start);
}



//------------------------ INSTRUMENTER :: GET ADDRESS -------------------------
template<>
inline
ADDRINT
Instrumenter<INS >::GetAddress
(
    INS x
)
{
    return INS_Address(x);
}



template<>
inline
ADDRINT
Instrumenter<IMG >::GetAddress
(
    IMG x
)
{
    return IMG_StartAddress(x);
}



template<>
inline
ADDRINT
Instrumenter<TRACE >::GetAddress
(
    TRACE x
)
{
    return TRACE_Address(x);
}
