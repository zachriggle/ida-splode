#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Contains routines for manipulating names
"""
import ida, addr, database
import re, collections

def DemangleShort(s,_=0):
    s = str(s)
    return ida.Demangle(s,ida.GetLongPrm(ida.INF_SHORT_DN)) or s

def DemangleLong(s,_=0):
    s = str(s)
    return ida.Demangle(s,ida.GetLongPrm(ida.INF_LONG_DN))  or s

def From(*args,**kwargs):
    return ToName(*args,**kwargs)

def ToName(x,Demangler=DemangleShort):
    """
    Given X, which is either a symbol triplet or integer, give it
    a name if possible.  Otherwise, is a pass-through to addr.Clickable

    >>> ToAddress(0x1391292)
    _main+192h
    >>> ToName(('msvcrt','malloc',15))
    msvcr!malloc+0Fh
    >>> ToName(3)
    00000003
    """
    # print x
    # print hasattr(x,'__len__')
    # try: print len(x)
    # except: pass
    if type(x) in (int,long):
        # If the address is within the bounds of the file loaded in IDA,
        # Attempt to resolve in IDA.  First try an exact name
        # (offset zero) and then just walk backward until we
        # find something.
        if addr.Min() <= x and x < addr.Max():
            FuncOffsetWithArgs = ida.GetFuncOffset(x)
            if FuncOffsetWithArgs:
                return re.sub('\([^)]+\)', '', FuncOffsetWithArgs)

            for Offset in xrange(0,256):
                NameInIDA = ida.NameEx(ida.BADADDR,x - Offset)
                NameInIDA = Demangler(NameInIDA)
                if NameInIDA:
                    return NameInIDA + MakeOffset(Offset)
        # Try to find another module within which the address belongs,
        # and state it as an offset from the base of the module.
        # If the address is not within any module, return the value provided
        # by addr.Clickable()
        else:
            for module in database.modules.find():
                Base = module['Base']
                End  = module['End']
                if Base <= x and x < End:
                    return "%s%s" % (module['Name'], MakeOffset(x-Base))
            return None # ??? addr.Clickable(x)

    elif type(x) in (str,unicode):
        return str(x)
    elif isinstance(x, collections.Iterable) and len(x) == 3:
        (module,routine,offset) = x
        rv = ''
        if   module and routine:  rv = module + "!" + routine
        elif module:              rv = module
        elif routine:             rv = routine
        if   offset:              rv += "%s" % MakeOffset(offset)
        return rv
    else:
        raise TypeError('Cannot convert %r to name' % x)

def StripName(x):
    """
    Remove characters from the string which IDA doesn't
    support as name characters, and replace them with placeholders.
    """
    Replacements = {' ': '_',
                    ':': '_',
                    '~': '_destructor_',
                    '=': '_eq_',
                    '==': '_equality_',
                    '<': '_le_',
                    '>': '_ge_',
                    ',': '_comma_',
                    '!': '_',
                    '+': '_plus_'}

    # Sorted by the length of the replaced string
    SortedReplacements = sorted(Replacements.iteritems(),
                                key=lambda x: len(x[0]),
                                reverse=True)

    for What,With in SortedReplacements:
        if What in x:
            x = x.replace(What,With)
    return x

def Offset(*args,**kwargs):
    return MakeOffset(*args, **kwargs)

def MakeOffset(x, sign=True):
    """Make integer x into an IDA-styled offset string

    >>> MakeOffset(0)
    0
    >>> MakeOffset(0xd0)
    0D0h
    >>> MakeOffset(0x1234)
    1234h
    """
    if sign and x == 0:
        return ""

    hexstr = "%X" % x
    if hexstr[0] in ('A','B','C','D','E','F'):
        hexstr = '0' + hexstr
    if x > 9:
        hexstr += 'h'

    if sign and x >= 0:
        return "+%s" % hexstr
    return hexstr

def Min():
    return ida.GetSegmentAttr(ida.FirstSegment(), ida.SEGATTR_START)

def Max():
    return ida.GetSegmentAttr(ida.LastSegment(), ida.SEGATTR_END)