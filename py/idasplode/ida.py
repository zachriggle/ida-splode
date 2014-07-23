#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Wrapper routines for IDA functionality, in addition
provides a single namespace for idc, idaapi, idautils.
"""

import addr

try:
    from idc import *
    from idc import __EA64__
    from idaapi import *
    from idautils import *
except:
    import sys,hashlib
    print "Could not import IDA functionality"
    __EA64__ = 0
    def GetInputFileMD5(): return hashlib.md5(file(GetInputFile()).read()).hexdigest().upper()
    def GetInputFile():    return sys.argv[-1]

def FirstSegment():
    return min(Segments())

def LastSegment():
    return max(Segments())

def IsInImage(x):
    return addr.Min() <= x and x < addr.Max()

def Poi(x):
    return Dword(x) if ptr_size == 4 else Qword(x)

def CodeXrefsFrom(EA):
    ref = Rfirst(EA)
    while ref != BADADDR:
        yield ref
        ref = Rnext(EA,ref)

PointerBytes = 8 if __EA64__ else 4

def StructMembers(sid):
    """
    Get a list of structure members information.

    @param sid: ID of the structure.

    @return: List of tuples (offset, name, size)

    @note: If 'sid' does not refer to a valid structure,
           an exception will be raised.
    """
    memqty = idc.GetMemberQty(sid)
    print 'Has %s members' % memqty

    if memqty == idaapi.BADADDR:
        raise Exception("No structure with ID: 0x%x" % sid)

    if memqty == 0:
        return
        yield

    off  = idc.GetFirstMember(sid)
    print "First at %r" % off

    size = idc.GetStrucSize(sid)
    print "First size %r" % size

    while off < size:
        print repr(name)
        name = GetMemberName(sid, off)
        if name is not None:
            print repr((off, name, idc.GetMemberSize(sid, off)))
            yield (off, name, idc.GetMemberSize(sid, off))
        print repr(off)
        off = idc.GetStrucNextOff(sid, off)



def GetFirstMember(sid):
    """
    Get offset of the first member of a structure

    @param sid: structure type ID

    @return: -1 if bad structure type ID is passed
             or structure has no members
             otherwise returns offset of the first member.

    @note: IDA allows 'holes' between members of a
           structure. It treats these 'holes'
           as unnamed arrays of bytes.

    @note: Union members are, in IDA's internals, located
           at subsequent byte offsets: member 0 -> offset 0x0,
           member 1 -> offset 0x1, etc...
    """
    s = idaapi.get_struc(sid)
    if not s or not s.memqty:
        return -1
    return idaapi.get_struc_first_offset(s)