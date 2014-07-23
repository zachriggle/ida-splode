#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Routines for displaying stack traces and stack variables.
"""

import ida, addr, name

def MakeBacktrace(Backtrace, heading=True):
    """
    @return Array of formatted trace strings

    Calls 'ToAddress' and 'ToName' on each item in 'x', each entry on its
    own line.

    Fixes in-module addresses so that instead of the return address, we get
    call site addresses.

    >>> MakeBacktrace([ScreenEA(), ScreenEA()-0x32])
    ['\t66500EFD CEPEditableDocTiffImpl::GetProductVersionString+2D',
    '\t66500ED0 CEPEditableDocTiffImpl::GetProductVersionString']

    Returns:
        Array of formatted strings.
    """
    #
    # Implementation note: Addresses in the backtrace are
    # for the return addr.  PrevHead() returns the previous
    # instruction, which should be the call site.
    #
    Callsites = [PrevHead(RetAddr) if ida.IsInImage(RetAddr) else RetAddr for RetAddr in Backtrace]
    return ["\t%s %s" % (addr.Clickable(addr.From(Site)), name.From(Site)) for Site in Callsites[:10]]

def MakeStackVarName(ea,off):
    """Given an routine address, fetch the name of the stack variable
    at the specified offset from the return address, and format it for
    display.

    >>> MakeStackVarName(ScreenEA(), 0x140)
    var_13C
    >>> MakeStackVarName(ScreenEA(), -4)
    (pushed arg)

    Returns:
        String name of the stack variable.
    """
    (Member,MemberOffset) = GetStackMemberContaining(ea,off)

    StackVarName = "???+" + name.Offset(off)
    if Member:
        StackVarName = ida.get_member_name(Member.id)

        if MemberOffset != 0:
            StackVarName += name.Offset(MemberOffset)
    elif MemberOffset < 0:
        # Currently, I have no way of enumerating stack-based argument
        # names, so we just return a generic name.
        StackVarName = "(pushed arg)"

    return StackVarName

def GetStackMemberContaining(ea,off):
    """
    @return Tuple containing (Nearest Stack Variable, Offset)

    Retrieve the name of a stack variable from the routine which
    contains address EA, where the stack variable is at offset 'off'
    from the initial stack pointer upon routine entry.

    For more information, see the source to IdaPython, as there are
    no documented APIs to do this.  Relevant source is for 'MakeLocal',
    at: https://code.google.com/p/idapython/source/browse/trunk/python/idc.py?r=333#855

    Consider the following frame, where we want to get the name for the
    stack variable that is 0x30 bytes from the return addr.

    -00000030 Zach            dd ?                    ; offset
    -0000002C FreeWilly       dd ?                    ; offset
    -00000028 b               dd ?                    ; offset
    -00000024 LooksLikeMemory dd ?
    -00000020 a               dd ?                    ; offset
    -0000001C xzibit          YoDawg ?
    -00000018 ms_exc          CPPEH_RECORD ?
    +00000000  s              db 4 dup(?)
    +00000004  r              db 4 dup(?)
    +00000008 argc            dd ?
    +0000000C argv            dd ?                    ; offset
    +00000010 envp            dd ?                    ; offset

    The variable we want is 'FreeWilly'.  Although IDA claims it is
    at offset 2C, this is based from the stack frame pointer, not the
    return addr.  Our offset is from RA.
    """

    # Helper for error message
    ErrLoc = "%s (%s)" % (addr.Clickable(ea), name.From(ea))

    # Returns a func_t*
    Func = ida.get_func(ea)

    # Returns a struc_t*
    Frame = ida.get_frame(Func)

    #
    # Find out how tall the frame is
    #
    # This is roughly the total size between $ra and the highest
    # stack variable.
    #
    FrameSize = ida.GetFrameSize(ea)

    #
    # Find the frame member whose EOFF is the same value
    # This will be the return address
    #
    ReturnAddr = None
    MemberCount = Frame.memqty

    for i in xrange(MemberCount):
        Member = Frame.get_member(i)
        Name   = ida.get_member_name(Member.id)
        # print repr(Name)
        # EOFF   = Member.eoff
        # if EOFF == FrameSize:
        if Name == " r":
            ReturnAddr = Member

    if not ReturnAddr:
        print "!!! Could not find return address in %s" % ErrLoc
        return (None, off)

    #
    # Calculate the offset from the return address,
    # and find the member with a matching EOFF
    #
    # IDA Stack frames look like this interanlly:
    #
    # ----------------- --.
    # local 1             |
    # -----------------   | Frame size
    # local 0             |
    # -----------------   |
    # frame pointer       |
    # -----------------   |
    # return address      |
    # ----------------- --'
    # arg0
    # -----------------
    # arg1
    #
    #
    # N.B. There may not be a matching value.  For example,
    #      a routine may 'lea ecx, [esp+xxx]' for a __thiscall,
    #      but never touch anything inside the large span of
    #      memory available for the structure.
    # Members   = [Frame.get_member(i) for i in xrange(MemberCount)]
    # StackOffs = [(m.soff, m.eoff)    for m in Members]

    DesiredOff = ReturnAddr.soff - off

    # print "ErrLoc    %s"    % ErrLoc
    # print "FrameSize %x"    % FrameSize
    # print "$ra       %x-%x" % (ReturnAddr.soff, ReturnAddr.eoff)
    # print "off       %x"    % off
    # print "size      %x"    % size
    # print "Desired   %x"    % DesiredOff

    rv = (None, off)
    if DesiredOff >= 0:
        Members = [Frame.get_member(i) for i in xrange(MemberCount)]
        SOFFs   = [M.soff for M in Members]
        EOFFs   = [M.eoff for M in Members]


        for i in range(MemberCount):
            Member = Members[i]
            Name   = ida.get_member_name(Member.id)
            sThis  = SOFFs[i]
            sNext  = EOFFs[i]

            try:    sNext = SOFFs[i+1]
            except: pass

            # print "%20s %x..%x" % (Name,sThis,sNext)

            if sThis <= DesiredOff and DesiredOff < sNext:
                # print "%x <= %x < %x" % (sThis, DesiredOff, sNext)
                rv = (Member, DesiredOff-sThis)

    else:
        rv = (None, DesiredOff)
    return rv
