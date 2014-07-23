#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Analysis for memory manipulation instructions instrumented by Pin
"""

import collections
from itertools   import groupby
from .. import ida, name, comment
from meta import Meta

#
# Routine to decorate an individual memory access
#
def GenerateComment(EA,Traces):
    """
    Example:
        ({'EA': 3471252,
          'EAMeta': {'Memory': True,
            'Stack': {'Entry': 0x13910e0, 'Offset':0x18}}
          'IP': 20517024,
          'Type': 'Read',
          'Value': 116195288,
          'ValueMeta': {'Heap': {'Base': 116195288,
            'Frames': (('MSVCR100', 'malloc', 0),
             20517011,
             20516998,
             20516998,
             20516998,
             20516998,
             20516998,
             20517527),
            'Size': 33},
           'Memory': True}},)
    """

    ReturnValue = ""

    # All of the traces will be homogenous, just check the
    # first one for what keys it has.
    KeysToInspect = [K for K in Meta.MemoryTypes if K in Traces[0]]
    Lines = []

    for Type in KeysToInspect:
        #
        # Initialize value to be printed
        #
        Lines.append('===== ' + Type + ' =====')

        #
        # Grab the 'Value/ValueMeta' and 'EA/EAMeta' pairs
        #
        # print Traces
        Metadata = [Meta(T[Type]) for T in Traces]
        # print Metadata

        if not Metadata:
            Lines.append("No metadata")
            continue

        #
        # Of the traces, determine which ones are memory operations.
        # Of those, determine which are heap/stack/symbol ops.
        #
        MemoryRefs  = filter(Meta.IsMemory,  Metadata)
        HeapRefs    = filter(Meta.IsHeap,    Metadata)
        StackRefs   = filter(Meta.IsStack,   Metadata)
        SymbolRefs  = filter(Meta.IsSymbol,  Metadata)
        ConstRefs   = filter(Meta.IsConst,   Metadata)
        UnknownRefs = filter(Meta.IsUnknown, Metadata)

        #
        # Easy check for things which we can't enhance.
        #
        # - Literal values (1,2,3)
        # - Unknown memory regions (eg VirtualAllocs)
        #
        if ConstRefs:
            Lines.append('=> Consts')
            Lines.append("    %s" % ", ".join(sorted(set(str(v) for v in ConstRefs))))

        if UnknownRefs:
            Lines.append('=> Unknown Pointers')
            Lines.append("    %s" % ", ".join(sorted(set(str(v) for v in UnknownRefs))))

        if HeapRefs:
            Lines.append("=> Heap Pointers")
            GroupedByBacktrace = groupby(HeapRefs, lambda M: M.Heap.Frames)
            for i, (Frames, Instances) in enumerate(GroupedByBacktrace):
                SizeOff   = [(I.Heap.Size, I.Heap.Offset) for I in Instances]

                # Meta-statistics about allocations that are being interacted with
                # at this instruction, all of which have the same stack trace for
                # the allocation.
                Lines.append("Count  Pctg  Offset Size")
                for (Size,Off), Count in collections.Counter(SizeOff).most_common():
                    Pctg = (100*Count)/len(HeapRefs)
                    Lines.append("%5i  %3i%%  %#6x %#x" % (Count, Pctg, Off, Size))

                # Dump out the lines, indented
                Lines.append("Backtrace:")
                # print Frames
                # print type(Frames)
                # print Frames[0]
                # print type(Frames[0])
                Lines += ("    %s %s" % (F,F.getComment())  for F in Frames[:8] if F.isImportant())

        if SymbolRefs:
            Lines.append("=> Symbols")
            Lines += sorted(set("    %s" % R.Symbol for R in SymbolRefs))



        if StackRefs:
            # If the stack references are all to a local-frame variable,
            # don't bother telling the user something that they can already
            # see.
            # XXX TODO

            Lines.append("=> Stack Pointers")
            Lines += sorted(set('    %s' % S for S in StackRefs))

    return '\n'.join(Lines)



def Freq(Counter, N=0):
    if len(Counter) <= N: return ''
    Val,Qty = Counter.most_common(N+1)[N]
    Total   = sum(Counter.values())

    if Qty == Total:
        return ''

    return "(%i%%)" % (Qty*100/Total)

def Summarize(EA,Traces):
    Metadata = [Meta(T['Value']) for T in Traces]

    MemoryRefs  = filter(Meta.IsMemory,  Metadata)
    HeapRefs    = filter(Meta.IsHeap,    Metadata)
    StackRefs   = filter(Meta.IsStack,   Metadata)
    SymbolRefs  = filter(Meta.IsSymbol,  Metadata)
    ConstRefs   = filter(Meta.IsConst,   Metadata)
    UnknownRefs = filter(Meta.IsUnknown, Metadata)

    Name    = None
    Counter = collections.Counter()

    #
    # Find out if we have any enriched metadata
    #
    if len(HeapRefs) == len(Metadata):
        Counter.update(M.Heap for M in Metadata)
    elif len(StackRefs) == len(Metadata):
        Counter.update(M.Stack for M in Metadata)
    elif len(SymbolRefs) == len(Metadata):
        Counter.update(M.Symbol for M in Metadata)

    # If we have enriched values, return the most common one
    if len(Counter):
        Value,N = Counter.most_common(1)[0]
        return "%s%s" % (Value, Freq(Counter))

    #
    # Otherwise, return the most common 5 values
    #
    Counter.update(M.Value for M in Metadata)
    Values = []
    for i,(V,N) in enumerate(Counter.most_common(5)):
        Values.append("%s%s" % (name.Offset(V,0), Freq(Counter, i)))
    return "%s" % ', '.join(Values)