#!/usr/bin/env python
# -*- coding: latin-1 -*-

import branch, memory, reconstruct, meta
from .. import ida, addr, database, query, comment
from collections import defaultdict

def MarkDynamicXrefs():
    #
    # Code-to-code cross references
    #
    # For these, we want to create the x-refs and add the comments
    # on the instructions
    #
    BranchAddresses = query.GetBranchAddresses()
    print len(BranchAddresses)

    for EA in query.GetBranchAddresses():
        comment.GenerateSummaryAnnotation(EA,Erase=False)
    return

    #
    # Code-to-data cross references
    #
    # For these, create the proper type of XREF
    #
    Types = {
        'Read': ida.dr_R,
        'Write': ida.dr_W
    }

    DataRefInstructions = set()
    RefsToAdd           = defaultdict(lambda: set())

    for Type,Flag in Types.items():
        query.TracesWhichModifyOrReadCurrentIdaFile
        Addr_to_Ins = query.AddressesTouched({'Type': Type})
        print len(Addr_to_Ins)
            # print Addr
            # print Insns
            # for Ins in Insns:
                # RefsToAdd[Ins].add((Ins, Addr, ida.XREF_USER | Flag))
                # DataRefInstructions.add(Ins)
                # ida.add_dref(Ins, Addr, ida.XREF_USER | Flag)
    # print RefsToAdd

    # for EA in DataRefInstructions:
        # comment.GenerateSummaryAnnotation(EA,Erase=False)


def ReconstructTypeAddr():
    reconstruct.ReconstructType('Address', ida.ScreenEA())

def ReconstructTypeValue():
    reconstruct.ReconstructType('Value', ida.ScreenEA())



def AnalyzeAddress(): pass
def RawData():
    print '\n'.join(repr(i) for i in query.GetTraces(ida.ScreenEA()))

def GoToNext(): pass
def MakeVtableStructs(): pass

def Summarize(EA, Traces):
    First = Traces[0]
    Type  = First['Type']

    TraceTypeHandlers = {
        'Branch': branch.Summarize,
        'Read': memory.Summarize,
        'Write': memory.Summarize
    }

    if Type not in TraceTypeHandlers:
        return "Instruction type %r not supported" % Type
    else:
        return TraceTypeHandlers[Type](EA,Traces)


def GenerateComment(EA,Traces):
    First = Traces[0]
    Type  = First['Type']

    TraceTypeHandlers = {
        'Branch': branch.GenerateComment,
        'Read': memory.GenerateComment,
        'Write': memory.GenerateComment
    }

    if Type not in TraceTypeHandlers:
        return "Instruction type %r not supported" % Type
    else:
        return TraceTypeHandlers[Type](EA,Traces)


def DumpHeapAllocations(EA, Traces):
    HeapTraces = query.TracesWhichInteractWithHeap()

    UniqueByBacktraceAndSize = {

    }
