#!/usr/bin/env python
# -*- coding: latin-1 -*-

from .. import ida, addr, database, query, comment, name, settings
from meta import Meta
import itertools


SizeToFlag = {
1: ida.FF_BYTE | ida.FF_DATA,
2: ida.FF_WORD | ida.FF_DATA,
4: ida.FF_DWRD | ida.FF_DATA,
8: ida.FF_QWRD | ida.FF_DATA
}

TypeToOperand = {
    # Read operations put the memory on the right-hand side
    'Read': 1,
    # Write operations put the memory on the left-hand side
    'Write': 0
}

class FieldInstance(object):
    def __init__(self, base, trace):
        """
        Arguments:
            base:           Base address of the allocation
            trace:          Full trace dictionary for the trace
        """
        self.base        = base
        self.instruction = trace['IP']
        self.type        = trace['Type']
        self.offset      = trace['Address']['Value'] - base
        self.valueMeta   = Meta(trace['Value'])
        self.size        = self.valueMeta.Size
        self.value       = self.valueMeta.Value

def GetHeapTraces(Field, EA):
    """
    Returns:
        Tuple containing:
        - Field name ('Address' vs 'Value')
        - Database cursor for all of the traces for the specified instruction
          which manipulate heap addresses.
    """
    Traces = query.GetTraces(EA, {'%s.Heap' % Field: {'$exists': 1}})

    if not Traces:
        print "No heap traces for %x's %s" % (EA, Field.upper())

    return Traces

def EnsureHeapMetadataHomogeneity(Metadata):
    """Ensures that all of the metadata provded are homoenous on the
    size, offset-from-base, and backtrace for all heap interactions.

    Returns:
        Tuple containing (size,offset,backtrace) for the common
        allocation type.
    """
    HeapMeta = tuple(M for M in Metadata if M.Heap)
    Sizes    = set(M.Heap.Size   for M in HeapMeta)
    Offsets  = set(M.Heap.Offset for M in HeapMeta)
    Frames   = set(M.Heap.Frames  for M in HeapMeta)

    if not len(HeapMeta):
        raise Exception("No heap data found")
    if len(HeapMeta) != len(Metadata):
        print "Not all interactions are heap metadata, only looking at heap data!"
    if len(Sizes) != 1:
        raise Exception("Multiple sizes %r, cannot analyze" % Sizes)
    if len(Offsets) != 1:
        raise Exception("Multiple offsets %r, cannot analyze" % Offsets)
    #if len(Frames) != 1:
    #    raise Exception("Multiple allocation stacks, cannot analyze")

    return (Sizes.pop(), Offsets.pop(), Frames)

def AskUserForStructOfSize(SizeHint):
    """Asks the user to either select an existing
    Returns:
        ida.BADNODE
    """
    #
    # Get the ID of the structure the user wants to use, or create
    # a new one.
    #
    struc = ida.choose_struc("Select Struc Size=%#x" % SizeHint)
    if struc:
        size = ida.GetStrucSize(struc.id)
        name = ida.GetStrucName(struc.id)
        if size == SizeHint or ida.AskYN(False, "Structure '%s' has size %x instead of %x; continue?" % (name, size, SizeHint)):
            return struc.id
    else:
        name = ida.AskStr("","Please enter a name for the new structure")
        if name:
            return ida.AddStruc(-1, name)

    print "Could not get/create structure"
    return ida.BADNODE

def TracesForHeapAllocation(BaseAddresses, Size, Frames):
    """
    Arguments:
        BaseAddresses: Iterable which contains allocation base addresses
        Size:          Allocation size to match
        Frames:        Allocation stack trace to match
    Return:
        Generator to iterate over traces with heap interactions
        where the heap allocation has the specified size and
        allocation trace.
    """
    #
    # For each trace that reads data *from* our allocation, or writes
    # *to* our location, we can discern three things:
    #
    # - The offset within the allocation
    # - The size of the field at that offset
    # - Some information on the data being written to that field
    #
    for Base in BaseAddresses:
        print "Base address %#x...\n" % Base

        for Trace in query.TracesWhichInteractWithMemory(trace_types=['Read','Write'],
                                                         value_types=['Address'],
                                                         low=Base, high=Base+Size):
            #
            # Skip all traces which don't match for the allocation size,
            # or the stack frames.
            #
            Heap = Meta(Trace['Address']).Heap
            if (not Heap) or (Heap.Frames not in Frames) or (Heap.Size != Size):
                continue

            #
            # Skip all traces that put us in memset/memcpy,
            # as these will generate false fields and field alignments
            #
            InsName = name.From(Trace['IP'])
            if 'memset' in InsName or 'memcpy' in InsName:
                # print "Mem(cpy|set)!"
                continue
            # print "!!! %s" % Trace
            yield Trace

def FieldFromTraces(StructId, TraceOffset, Traces):
    """Creates a new field in the provided structure, at the provided offset,
    by using information gleaned from the provided traces.

    Differs from ida.AddStrucMember in that it handles coalescing existing
    fields created with FieldFromTraces.

    Arguments:
        StructId:     IDA Pro id for the structure
        Offset:       Offset of the field to create
        Size:         Size of the field to create
    """
    #
    # Determine the largest manipulation of memory across the traces,
    # and determine the span of bytes within the structure that the
    # field would cover.
    #
    TraceSize  = max({t['Value']['Size'] for t in Traces})
    TraceBytes = set(range(TraceOffset,TraceOffset+TraceSize))

    #
    # Create a mapping of the existing structure's fields from
    # offset-to-bytes and vice versa.  For example, a two-DWORD
    # struct would look like:
    #
    # FieldToBytes = {0: [0,1,2,3], 4:[4,5,6,7]}
    # BytesToFIeld = {0:0, 1:0, 2:0, 3:0, 4:4, 5:4, 6:4, 7:4}
    #
    for s in ida.StructMembers(StructId):
        print repr(s)
    FieldToBytes = {o:set(range(o,o+s)) for (o,n,s) in ida.StructMembers(StructId)}
    BytesToField = {}
    for Offset,Bytes in FieldToBytes.items():
        for Byte in Bytes:
            BytesToField[Byte] = Offset

    #
    # Are ANY of the new field's bytes accounted for in existing fields?
    #
    ByteOverlap = set(B for B in TraceBytes if B in BytesToField)
    if ByteOverlap:
        #
        # Are all of the bytes in our member **contained** within one
        # existing member?
        #
        # If so, there's nothing that we need to do
        #
        for M,B in FieldToBytes.items():
            if TraceBytes.issubset(B):
                return

        #
        # We overlap at least some existing bytes, but we do not align perfectly with,
        # or fit inside of, any single existing field.
        #
        # This means that we would have to coalesce/grow smaller field(s) into a larger one.
        # As long as we don't start/stop in the middle of two fields, ask the user
        # if we can coalesce the fields together.
        #
        IntersectedFields = sorted(M for M,B in FieldToBytes.items() if TraceBytes.intersection(B))
        IntersectedBytes  = set(B for B,F in BytesToField.items() if F in IntersectedFields)
        IntersectedNames  = tuple(ida.GetMemberName(StructId, M) for M in IntersectedFields)

        #
        # Of the fields we intersect, if our candidate field does not encompass
        # all of the bytes of those fields, bail out and use the raw offset.
        #
        if not TraceBytes.issuperset(IntersectedBytes):
            return

        #
        # We can cleanly coalesce all of the intersected fields into one larger field.
        # If any of the fields are NOT auto-generated (determined by the name) then
        # ask the user for permission.
        #
        #
        if not all(N.startswith(settings.RECONSTRUCTED_FIELD_NAME_PREFIX) for N in IntersectedNames):
            CommaSeparatedNames = ', '.join(IntersectedNames)
            if not ida.AskYN(False, 'Coalesce fields %s into single field?' % CommaSeparatedNames):
                print "Field at %x (%x bytes) overlaps with fields %s" % (TraceOffset, TraceSize, CommaSeparatedNames)
                return

        for Field in IntersectedFields:
            ida.DelStrucMember(StructId, Field)


    if 0 != ida.AddStrucMember(sid=StructId,
                           name='autofield_%x' % TraceOffset,
                           offset=TraceOffset,
                           flag=SizeToFlag[TraceSize],
                           typeid=-1,
                           nbytes=TraceSize):
        print "Could not create field!"


def ReconstructType(Field, EA):
    #
    # Get at the metadata for *this* instruction alone
    #
    Traces                 = GetHeapTraces(Field, EA)
    Metadata               = tuple(Meta(Trace[Field]) for Trace in Traces)
    (Size, Offset, Frames) = EnsureHeapMetadataHomogeneity(Metadata)
    StructId               = AskUserForStructOfSize(Size)
    BaseAddresses          = set(M.Heap.Base for M in Metadata)

    if StructId == ida.BADNODE:
        return

    #
    # Presumably, this instruction was hit multiple times, and may have
    # operated on several different allocations of the exact same variety.
    #
    # Given this data, find *all* instructions which interacted with
    # the exact same data.  Specifically, we want to get at the traces.
    #
    for F in Frames:
        print ", ".join(str(_) for _ in Frames)
    # print "Analyzing allocations of size %#x with call frame..." % Size
    # print '\n'.join('    %s' % F for F in Frames)

    RelevantTraces = tuple(TracesForHeapAllocation(BaseAddresses, Size, Frames))
    UniqueOffset   = lambda T: T['Address']['Heap']['Offset']

    print "Found %#x traces" % len(RelevantTraces)

    #
    # Group all of the fields by their offset from the base
    # of the heap allocations
    #
    for TraceOffset, Traces in itertools.groupby(RelevantTraces, UniqueOffset):
        # N.B. itertools.groupby() group can only be iterated once
        #      so we have to save it off.
        Traces = tuple(Traces)
        FieldFromTraces(StructId, TraceOffset, Traces)

        #
        # Group the traces by instruction, and set the structure type
        # on the memory operand if there isn't one already.
        #
        for Trace in Traces: # o_displ
            EA       = Trace['IP']
            Operands = { 0: ida.GetOpType(EA, 0),
                         1: ida.GetOpType(EA, 1)}

            # We want to find the 'Address' operand, which will
            # be of type displacement, memory, or phrase.
            OpMem   = next(k for k,v in Operands.items() if v in (ida.o_displ, ida.o_mem, ida.o_phrase))

            OpValue = ida.GetOperandValue(EA, OpMem)
            OpOff   = 0

            # o_phrase comes up for some instructions that look like:
            #       mov reg, [reg2]
            # and OpValue will then be an index for the list returned by GetRegisterList()
            # instead of '0'.
            if Operands[OpMem] == ida.o_phrase:
                OpOff = TraceOffset
            else:
                OpOff = TraceOffset - OpValue
                print "%x = %x - %x" % (OpOff, TraceOffset, OpValue)

            print "OpStroffEx(%x, %x, %x, %x)" % (EA, OpMem, StructId, OpOff)
            ida.OpStroffEx(EA, OpMem, StructId, OpOff)
