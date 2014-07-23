#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Contains routines for manipulating addresses
"""

import ida, name

def Min():      return ida.GetSegmentAttr(ida.FirstSegment(), ida.SEGATTR_START)
def Max():      return ida.GetSegmentAttr(ida.LastSegment(),  ida.SEGATTR_END)


def ModuleImports():
    """Returns:
        List of all imported routines from the import section
    """
    Imports = {}

    def Callback(ea, n, ordinal):
        n = name.DemangleShort(n)
        Imports.update({n:ea})
        return 1

    for n in xrange(ida.get_import_module_qty()):
        ida.enum_import_names(n, Callback)

    return Imports

def From(*args,**kwargs):
    return ToAddress(*args,**kwargs)

def ToAddress(x):
    """Makes a best offort to conver the specified address into
    a string name.

    >>> hex(ToAddress(15))
    0xf
    >>> ToAddress(('msvcrt','malloc',0))
    0xffffffff
    >>> ToAddress('DllMain')
    0x12345 # Address of DllMain

    Returns:
        Address of named byte/instruction, or ida.BADADDR
    """
    if type(x) in (int,long):
        return int(x)

    #
    # If not an int or long, expect that we've been given a
    # symbol triplet of (module,routine,offset).
    #
    Module   = str(x[0]).lower()
    Routine  = str(x[1])
    SRoutine = name.StripName(Routine)
    LRoutine = Routine.lower()

    #
    # Candidate names, processed in order
    #
    Candidates = [
        Routine,                # Exact match
        SRoutine,               # Exact match, sans unmatchable characters
        SRoutine.strip('Stub')  # Strip off 'Stub' sometimes appended to routine name
    ]

    #
    # Try exact matches in candidates
    #
    for Name in Candidates:
        Address = ida.LocByName(str(Name))
        if Address != ida.BADADDR:
            return Address

    #
    # Try imports next, knocking everything to lowercase
    #
    for Name,Addr in ModuleImports().items():
        NameLow = Name.lower()
        if NameLow in LRoutine or LRoutine in NameLow:
            return Addr

    #
    # Finally, try all names that IDA knows about
    #
    for Address,Name in ida.Names():
        if any([Routine in Name, SRoutine in Name, LRoutine in Name]):
            return Address

    #
    # Default to ida.BADADDR
    #
    return ida.BADADDR

def Clickable(x):
    """If possible, make the address x clickable

    Return:
        A clickable string, or x
    """
    width = ida.PointerBytes * 2
    if x == ida.BADADDR:
        fmt = "{:=^%is}" % width
        return fmt.format("BADADDR")
    if type(x) in (int,long):
        fmt = "%%0%iX" % width
        return fmt % x
    return x