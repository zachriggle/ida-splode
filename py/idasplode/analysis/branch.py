#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Analysis for branching instructions
"""
import collections, datetime
from .. import dynamic, name, addr, ida
from meta import Symbol

class Bench(object):
    def __init__(self):
        self.last = self.now()

    def now(self):
        return datetime.datetime.now()

    def bench(self, x):
        now = self.now()
        # if self.last != 0:
            # print "%r - %s" % (now - self.last, x)
        self.last = now


def GenerateComment(EA,Traces):
    ReturnValue = "(no dynamic targets)"

    # Skip imports
    if  2 == ida.GetOpType(EA, 0):
        ConstTarget = ida.GetOperandValue(EA, 0)
        if '.idata' == ida.SegName(ConstTarget):
            return "%s" % Symbol(ConstTarget)

    Targets           = [Symbol(Trace['Target']) for Trace  in Traces]
    TargetCount       = collections.Counter()
    UniqueTargets     = set(Targets)
    DynamicTargets    = set()
    TargetCount.update(Targets)


    for Target in sorted(UniqueTargets):
        Address = addr.From(Target.Val)
        Count   = TargetCount[Target]

        # Skip imports
        # if SegName(Target) == '.idata':
            # continue

        # If there is no stub to create an x-ref,
        # create one with the proper name
        # if Address == ida.BADADDR:
        if Address == ida.BADADDR:
            # print "Address is bad, trying..."
            ReturnAddr = ida.NextHead(EA)
            # print 'ReturnAddr: %r' % ReturnAddr
            Callee     = ida.get_func(ReturnAddr)
            # print 'Callee: %r' % Callee
            SpMove     = ida.get_sp_delta(Callee, ReturnAddr)
            SpMove     -= 4 # Return address already accounted for
            # print 'SpMove: %r' % SpMove
            NewRoutineName = name.StripName(name.From(Target.Val))
            # print 'NewRoutineName: %r' % NewRoutineName

            # Update address and target
            Address    = dynamic.CreateDynamicFunctionStub(NewRoutineName, SpMove)
            # print 'Address: %r' % Address
            Target     = Symbol(Target.Val)
            # print 'Target: %r' % Target

        DynamicTargets.add("%3i %s" % (Count, Target))

        # Create the code xref if one does not
        # already exist (e.g. auto-created by IDA)
        if Address not in ida.CodeXrefsFrom(EA):
            Flag = 0
            Mnem = ida.GetMnem(EA)
            if Mnem.startswith('c'):
                Flag = ida.fl_CN
            if Mnem.startswith('j'):
                Flag = ida.fl_JN

            ida.AddCodeXref(EA, Address, ida.XREF_USER | Flag)


    if DynamicTargets:
        ReturnValue = "\n"
        ReturnValue += "\n".join(DynamicTargets)

        if len(UniqueTargets) > 1:
            ReturnValue += '\n-----------------'
            ReturnValue += "\n%3i total hits" % len(Targets)

    return ReturnValue

def Summarize(*args): return GenerateComment(*args)