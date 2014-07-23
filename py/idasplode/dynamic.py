#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Contains routines for creating stubs for dynamically-resolved routines,
so that there's at least an address to point at for x-refs.

All of these routines are stored in a dynamically-created segment.
"""


import ida, segment, settings, addr


def FillSegmentWithNops(x):
    """
    Sets every byte in the segment that contains 'x' to a
    NOP by changing its value to 0x90 and marking it as code.
    """
    for Byte in segment.SegmentAddresses(x):
        ida.patch_byte(Byte, 0x90)
        ida.MakeCode(Byte)

def GetDynamicSegment(Name = settings.SECTION_NAME_DYNAMIC):
    """
    @return Address of the IdaSplode segment; this segment
    is created if it does not exist.
    """
    print "Name: %r" % Name
    Segment = ida.SegByName(Name)
    print "Segment: %r" % Segment

    if Segment == ida.BADADDR \
    or Segment == 0:

        Begin = addr.Max()+0x10000
        End   = Begin     +0x10000

        if ida.SegCreate(Begin, End, 0, 1, 0, 0):
            ida.SegRename(Begin, Name)
            FillSegmentWithNops(Begin)
            return Begin

    return Segment

def FindFirstNop(x):
    """
    @return Address of the first byte with the value 0x90
    between 'x' and the end of the segment, or BADADDR.
    """
    for Addr in segment.SegmentAddresses(x):
        if 0x90 == ida.get_byte(Addr):
            return Addr
    return ida.BADADDR


def CreateDynamicFunctionStub(Name,StackDelta):
    """Create a function stub for a dynamically resolved routine so that IDA can track X-refs"""
    Seg  = GetDynamicSegment()
    print "Seg: %r" % Seg
    Ea   = FindFirstNop(Seg)
    print "Ea: %r" % Ea
    Name = str(Name)
    print "Name: %r" % Name
    Bytes = [0xc2, StackDelta, 0x00]
    for i,Byte in enumerate(Bytes):
        ida.patch_byte(Ea+i, Byte)
        ida.MakeCode(Ea+i)
    ida.MakeName(Ea,Name)
    ida.MakeFunction(Ea, Ea+len(Bytes))
    return Ea
