#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Common query helpers for MongoDB
"""

import ida, database, addr
from collections import defaultdict

def Reads(query={}):
    """
    Returns:
        cursor of all read traces
    """
    query = {'$and': [query, {'Type':'Read'}]}
    return database.traces.find(query)

def Writes(query={}):
    """
    Returns:
        cursor of all write traces
    """
    query = {'$and': [query, {'Type':'Write'}]}
    return database.traces.find(query)

def TracesWhichInteractWithMemory(trace_types=['Read','Write'], value_types=['Address','Value'], low=0, high=ida.BADADDR, query={}):
    """
    Wrapper function which allows us to query for specific traces that
    interact with memory in the provided manner.

    By default, produces the following query:
        {'$and': [{},
          {'$or': [{'Type': 'Read'}, {'Type': 'Write'}]},
          {'$or': [{'$and': [{'Address.Value': {'$gte': 0}}, {'Address.Value': {'$lt:': 4294967295L}}]},
                   {'$and': [{'Value.Value': {'$gte': 0}}, {'Value.Value': {'$lt:': 4294967295L}}]}]}]}

    Args:
        trace_types:  Tuple containing types of traces to inspect (e.g. ['Read','Write'])
        value_types:  Tuple containing value types to inspect (e.g. ['Address','Value'])
        low:          Lower address bound
        high:         Upper address bound
        query:        Additional manual query

    Returns:
        Database cursor
    """

    query = {'$and': [
            #
            # Include the user-specified query
            #
            query,

            #
            # Filter by trace type (read/write)
            #
            {'$or':[
                {'Type':t} for t in trace_types
            ]},

            #
            # Filter by value type (effective address, or value read)
            #
            {'$or':[
                {'$and': [{'%s.Value' % t: {'$gte': low}},
                         {'%s.Value' % t: {'$lt': high}}
                ]} for t in value_types
            ]}
        ]}
    return database.traces.find(query)


def TracesWhichInteractWithHeap(query={}):
    query = {'$and': [query, {
        {'$or':[ {'%s.Heap' % ValueType: {'$exists': True}}
                for ValueType in ['Address','Value']]}
    }]}
    return database.traces.find(query)


def TracesWhichModifyOrReadCurrentIdaFile(query={}):
    """
    Queries for all Read and Write traces, where either the
    memory read, or the value at the memory address read,
    is within the file open in IDA.
    """
    return TracesWhichInteractWithMemory(low=addr.Min(), high=addr.Max())

def AddressesTouched(query={}):
    """
    Returns:
        dict() of all addresses in the currently open file
        which were read/written during the trace.

        dict is of format {data: set(ins, ins, ins)} where
        'data' is the address of the data reference, and
        'ins' is the instruction referencing it.
    """
    Addresses = defaultdict(lambda: set())

    for T in TracesWhichModifyOrReadCurrentIdaFile(query):
        IP = T['IP']
        for Candidate in (T['Value'], T['Address']):
            V  = Candidate['Value']
            if ida.IsInImage(V):
                Addresses[V].add(IP)

    return Addresses

# def Foo():
#     """
#     Returns:
#         List of tuples
#     """

def GetTraces(ea, query={}):
    """Retrieve all traces in the database for the given EA

    Args:
        query: Additional collection.find() query arguments

    Returns:
        A database cursor
    """
    database.traces.ensure_index('IP')
    query = {'$and': [query, {'IP':ea, 'Type': {"$ne": "Seen"}}]}
    return tuple(database.traces.find(query))

def GetBranches(query={}):
    query = {'$and': [query, {'Type': 'Branch'}]}
    return database.traces.find(query)

def GetBranchAddresses(query={}):
    return set(int(T['IP']) for T in GetBranches())

def GetEnhancedInstructions(query={}):
    """Retrive list of instruction addresses with metadata.

    Args:
        query: Additional collection.find() query arguments

    Returns:
        A database cursor
    """
    query = {'$and': [query, {'Type': {"$ne": "Seen"}}]}
    Hits  = database.traces.find(query, fields={'IP': 1})
    Hits  = set(int(h['IP']) for h in Hits)
    # print 'Enhanced: %s' % ', '.join(hex(i) for i in Hits)
    return Hits

def GetHitInstructions(query={}):
    """Retrieve list of instruction addresses JITed by Pin, which is close
    enough to the instructions executed to not care.


    Args:
        query: Additional collection.find() query arguments

    Returns:
        A database cursor
    """
    query = {'$and': [query, {'Type': 'Seen'}]}
    Hits = database.traces.find(query, fields={'IP': 1})
    Hits = set(int(h['IP']) for h in Hits)
    # print 'JITed: %s' %  ', '.join(hex(i) for i in Hits)
    return Hits

def GoToNextEnhancedInstruction():
    """Jump to the next instruction enhanced by PIN"""
    EA = ida.ScreenEA()

    while EA != ida.BADADDR:
        EA = ida.NextHead(EA)
        if GetTraces(EA).count():
            ida.Jump(EA)
            return

def DumpAddress():
    print '\n'.join(["%s" % t for t in GetTraces(ida.ScreenEA())])

