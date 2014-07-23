#pragma once
/*
 * MemoryMetadata.h
 *
 *  Created on: Oct 22, 2013
 */

#include "Common.h"



/**
 * Metadata functor.  Exists only to have a clean mnemonic for ostream operator<<.
 *
 * Usage example:
 * @code
 * // Define specialization class
 * class MyMetadataProvider : public MetadataProvider { ... }
 *
 * // Instantiate specialization class
 * new MyMetadataProvider;
 *
 * // Iterates over all metadata providers,
 * // and dumps the information.
 * cerr << Metadata(0xdeadbeef, 4) << endl;
 * @endcode
 */
class AddressMetadata
{
public:
    ADDRINT Value;
    ADDRINT Size;
    //    AddressMetadata(ADDRINT Value, ADDRINT Size);
    AddressMetadata
    (
        ADDRINT Value_,
        ADDRINT Size_
    )  : Value(Value_),
         Size(Size_)
    {}



};

/** Metadata provider interface */
class MetadataProvider : public virtual SortableMixin
{
public:
    static set<MetadataProvider *> Instances;
    MetadataProvider
    (
    );
    virtual
    ~MetadataProvider
    (
    );
    virtual bool
    Enhance
    (
        ostream    &os,
        ADDRINT     Value,
        ADDRINT     Size
    ) const = 0;


};

/**
 * Provides information about the stack frame to which Value belongs
 */
class StackFrameMetadataProvider : public MetadataProvider
{
public:
    StackFrameMetadataProvider
    (
    );
    bool
    Enhance
    (
        ostream    &os,
        ADDRINT     Value,
        ADDRINT     Size
    ) const;


};

/**
 * Provides information about readable text located at an address
 */
class TextualMetadataProvider : public MetadataProvider
{
public:
    TextualMetadataProvider
    (
    );
    bool
    Enhance
    (
        ostream    &os,
        ADDRINT     Value,
        ADDRINT     Size
    ) const;
};

/**
 * Provides information about the heap allocation to which Value belongs
 */
class HeapAllocationMetadataProvider : public MetadataProvider
{
public:
    HeapAllocationMetadataProvider
    (
    );
    bool
    Enhance
    (
        ostream    &os,
        ADDRINT     Value,
        ADDRINT     Size
    ) const;
};

/**
 * Provides information about the debug symbol to which Value refers
 */
class SymbolMetadataProvider : public MetadataProvider
{
public:
    SymbolMetadataProvider
    (
    );
    bool
    Enhance
    (
        ostream    &os,
        ADDRINT     Value,
        ADDRINT     Size
    ) const;


};



ostream&
operator<<
(
    ostream               & os,
    const AddressMetadata & mm
);
