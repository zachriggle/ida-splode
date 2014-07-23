#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Routines for reconstructing structures based on recorded manipulations
"""

from .. import query
from meta import Meta

def ReconStructHeap(EA):
    #
    # For the current instruction, find all heap references
    #
    Traces = query.GetTraces(EA)

    if not Traces:
        print "No heap traces for %x" % EA
        return

    #
    # Get at the metadata
    #
    Metadata = [Meta(Trace['Address']) for Trace in Traces]


    #
    # Ensure homogoneity of:
    #    - Allocation type
    #    - Allocation size
    #    - Allocation offset
    #    - Allocation trace
    #
    HeapMeta = tuple(M for M in Metadata if M.Heap)
    Sizes    = set(M.Size   for M in HeapMeta)
    Offsets  = set(M.Offset for M in HeapMeta)
    Traces   = set(M.Trace  for M in HeapMeta)

    if len(HeapMeta) != len(Metadata)
        print "Non-Fatal: Not all interactions are heap metadata!"
    if len(Sizes) != 1:
        print "Fatal: Multiple sizes %r" % Sizes
    if len(Offsets) != 1:
        print "Fatal: Multiple offsets %r" % Offsets
    if len(Traces) != 1:
        print "Fatal: Multiple allocation stacks"



