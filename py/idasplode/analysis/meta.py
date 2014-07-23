#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Metadata manipulation and data extraction routines.
"""
from .. import stack, name, addr, ida, comment

IrrelevantAllocationFrames = (
  "malloc",
  "operator new",
  "GlobalAlloc",
  "LocalAlloc",
  "HeapAlloc",
  "calloc",
  "realloc",
  "RtlInitializeExceptionChain",
  "BaseThreadInit",
  "__tmain"
)

class Meta():
  MemoryTypes = ('Address','Value')
  BranchTypes = ('Branch')

  """
  Helper class to provide structured (rather than dict keys) access
  to metadata logged by the pintool.
  """
  def __init__(self, d):

    self.Value   = d.get('Value',0)
    self.Size    = d.get('Size',0)
    self.Memory  = d.get('Memory', False)
    self.Heap    = None
    self.Stack   = None
    self.Symbol  = None
    self.name    = name.From(self.Symbol) if self.IsSymbol() else name.From(self.Value)

    if 'Stack'  in d: self.Stack   = Stack(d['Stack'])
    if 'Symbol' in d: self.Symbol  = Symbol(d['Symbol'])
    if 'Heap'   in d: self.Heap    = Heap(d['Heap'])

    #
    # Symbolic resolution for whitelisted addresses doesn't occur
    # at runtime, so there won't be a 'Symbol' key for these.
    #
    if not self.Symbol and addr.Min() <= self.Value and self.Value < addr.Max():
      self.Symbol = Symbol(self.Value)


  def IsHeap(self):
    return bool(self.Heap)

  def IsStack(self):
    return bool(self.Stack)

  def IsSymbol(self):
    return bool(self.Symbol)

  def IsMemory(self):
    return bool(self.Memory)

  def IsConst(self):
    return not self.Memory

  def IsUnknown(self):
    return self.Memory and not (self.IsHeap() or self.IsSymbol() or self.IsStack())

  def __int__(self):
    return self.Value

  def __lt__(self, other):
    if type(self) == type(other):
      return self.Value < other.Value
    else:
      return self.Value < other

  def __str__(self):
    if self.Symbol: return str(self.Symbol)
    if self.Stack:  return str(self.Stack)
    if self.Heap:   return str(self.Heap)
    return name.From(self.Value) or name.Offset(self.Value, False)

  def __hash__(self):
    if self.Symbol: return hash(str(self.Symbol))
    if self.Stack:  return hash(str(self.Stack))
    if self.Heap:   return hash(str(self.Heap))
    return hash(self.Value)

class Symbol(object):
  """
  Helper to convert a (module, routine, offset) triplet into
  a printable symbol

  >>> s = Symbol(['SHLWAPI', 'CFileStream::AddRef', 13])
  >>> print s
  SHLWAPI!CFileStream::AddRef+0Dh
  """
  def __init__(self, v):
    self.Val = v
  def __str__(self):
    EA = addr.From(self.Val)

    # 'Addr' is the return address; modify to be call address
    # by going back one instruction
    # if ida.IsInImage(EA):
      # EA = ida.PrevHead(EA)

    return "%s %s" % (addr.Clickable(EA), name.From(self.Val))
  def __repr__(self):
    return "Symbol(%r)" % self.Val
  def __eq__(self, other):
    return hash(self) == hash(other)
  def __hash__(self):
    return hash(str(self))
  def isImportant(self):
    strSelf       = str(self)
    if any(Alloc in strSelf for Alloc in IrrelevantAllocationFrames):
      return False
    return True
  def getComment(self):
    print repr(self.Val)
    if type(self.Val) in (int,long):
      if comment.HasUserComment(self.Val):
        C = ida.CommentEx(self.Val, 1) or ida.CommentEx(self.Val, 0) or ""
        if C:
          return "; %s" % C
    return ""

class Backtrace(tuple):
  """
  Helper class to convert a list of return addresses into list of symbols
  """
  def __init__(self, v):
    # print v
    # print type(v)
    # print [i for i in v]
    # print [Symbol(i) for i in v]
    super(Backtrace,self).__init__(Symbols)

class Heap():
  """
  Contains data for 'ValueWritten' in the trace below.

  {'Counter': 51,
                 'IP': 1760720479,
                 'Type': 'Write',
                 'ValueWritten': {'Heap': {'Base': 208732032,
                                           'Frames': [['msvcrt', 'malloc', 0],
                                                      1760720478,
                                                      1760720676,
                                                      1760721060,
                                                      ['ntdll', 'LdrpCallInitRoutine', 0],
                                                      ['ntdll',
                                                       'LdrpRunInitializeRoutines',
                                                       623]],
                                           'Offset': 0,
                                           'Size': 128},
                                  'Memory': True,
                                  'Size': 4,
                                  'Value': 208732032}}
  """
  def __init__(self,d):
    # print d
    self.Base    = d['Base']
    self.Frames  = tuple(Symbol(f if type(f) not in (int,long) else ida.PrevHead(f)) for f in d['Frames'])
    # print "!!!!!!!"
    # print self.Frames
    self.Offset  = d['Offset']
    self.Size    = d['Size']
    self.Val     = d
  def __hash__(self):
    return hash((self.Frames, self.Offset, self.Size))
  def __eq__(self, other):
    return hash(self) == hash(other)
  def __repr__(self):
    return "Heap(%r)" % self.Val
  def FirstImportantFrame(self):
    return next((F for F in self.Frames if F.isImportant()), Symbol(ida.BADADDR))
  def __str__(self):
    return "%s bytes into %s-byte alloc from\n%s" % (name.Offset(self.Offset,0), name.Offset(self.Size,0), self.FirstImportantFrame())


class Stack():
  """
  Contains data for 'ValueWritten.Stack' in the trace below.

  {'Counter': 72,
                 'IP': 1760722106,
                 'Type': 'Write',
                 'ValueWritten': {'Memory': True,
                                  'Size': 4,
                                  'Stack': {'Entry': 1760722044, 'Offset': 8},
                                  'Value': 448700}}

  >>> s = Stack({'Entry': 1760722044, 'Offset': 8})
  >>> print s
  00001234 FooBar var_4
  """
  def __init__(self,d):
    self.Val     = d
    self.RtnEntry   = d['Entry']
    self.Offset  = d['Offset']
  def __str__(self):
    # If 'Entry' is an address, this implies that it's within the
    # file that we have open (i.e., not a symbol triplet).
    StackvarName = name.Offset(self.Offset)

    if type(self.RtnEntry) in (int,long): # and addr.Min() < self.Entry and self.Entry < addr.Max():
        StackvarName = stack.MakeStackVarName(self.RtnEntry,self.Offset)

    # If 'Entry' is a triplet (e.g. ['msvcrt', 'HeapAlloc', 13]) then we
    # cannot perform a lookup in IDA.
    # else type(Entry) is tuple:
    return "%s %s %s" % (addr.Clickable(addr.From(self.RtnEntry)), name.From(self.RtnEntry), StackvarName)


    return stack.MakeStackVarName(self.Entry, self.Offset)
  def __hash__(self):
    return hash((self.RtnEntry, self.Offset))
  def __eq__(self, other):
    return hash(self) == hash(other)
  def __repr__(self, other):
    return "Stack(%r)" % self.Val