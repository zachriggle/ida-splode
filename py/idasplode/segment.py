#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Contains routines for manipulating segments.
"""

import ida

def GetSegByName(name):
    """
    @return Address of the first byte in the Segment
    with the provided name, or BADADDR
    """
    for Segment in ida.Segments():
        if ida.SegName(Segment) == name:
            return Segment
    return ida.BADADDR


def SegmentAddresses(s):
    """
    Generator that iterates over every address in the specified segment.
    """
    for Byte in range(ida.SegStart(s), ida.SegEnd(s)):
        yield Byte
