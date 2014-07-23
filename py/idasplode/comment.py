#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Routines for annotating individual addresses.

In order to avoid trampling on user comments, and to distinguish them from IdaSplode-
generated annotations, all annotations are prefixed with a marker.

Additionally, users are prompted if an annotation would overwrite existing data that
is not an annotation.
"""

import ida, textwrap, database, analysis, query, addr

IDASPLODE_ANNOTATION_MARKER = "><"

def Wrap(x):
    """Wraps the provided text to 80 characters without stripping whitespace"""
    if type(x) is not str:
        x = str(x)


    lines_in = x.split('\n')
    lines_out = []

    for line in lines_in:
        lines_out += textwrap.wrap(line,width=80, replace_whitespace=False)

    return str('\n'.join(lines_out))

def Banner(x):
    """Turn x into a banner

    >>> repr(Banner('Tada'))
    '\n=============== T a d a ================\n'
    """
    return "\n{:=^40s}\n".format(' ' + ' '.join(x) + ' ')

def HasUserComment(ea):
    """Return True if the user has overridden the annotation for the specified address"""
    return (ida.Comment(ea) and not GetAnnotation(ea))

def RemoveAnnotation(ea):
    """Removes the annotation on the provided address"""
    ida.MakeComm(ea,'')

def SetAnnotation(ea, comment, force=False):
    """Sets the annotation on the specified address"""
    if force or not HasUserComment(ea) or 1 == ida.AskYN(False, 'Overwrite comment?'):
        RemoveAnnotation(ea)
        Prefix = IDASPLODE_ANNOTATION_MARKER

        # For single-line comments, fit them on the same line as the marker
        # otherwise, split multi-line comments onto a separate line from
        # the marker.
        Prefix += '\n' if '\n' in comment else ' '
        print comment
        ida.MakeComm(ea,Prefix + comment)

def GetAnnotation(ea):
    """Retrieves the current annotation for the specified address"""
    Comment = ida.Comment(ea)
    if Comment and Comment.startswith(IDASPLODE_ANNOTATION_MARKER):
        return Comment
    return ""

def RefreshAnnotations():
    """Refresh all annotations"""
    for Head in ida.Heads():
        if GetAnnotation(Head):
            analysis.AnalyzeAddress(Head,Erase=False)

def GenerateAnnotationCommon(CommentGenerator, EA, Erase, PrintTraceCount):
    """Analyze the specified address, and add information to its comment"""
    EA  = EA or ida.ScreenEA()

    # Toggle annotation on successive presses
    if Erase and GetAnnotation(EA):
        RemoveAnnotation(EA)
        return

    # Add information for traces
    Traces    = [t for t in query.GetTraces(EA)]
    if PrintTraceCount:
        print "%s has %i traces" % (addr.Clickable(EA), len(Traces))

    if Traces:
        WrappedComment   = Wrap(CommentGenerator(EA,Traces))
        SetAnnotation(EA, WrappedComment)

def GenerateSummaryAnnotation(EA=None, Erase=True, PrintTraceCount=True):
    GenerateAnnotationCommon(analysis.Summarize, EA, Erase, PrintTraceCount)

def GenerateDetailedAnnotation(EA=None, Erase=True, PrintTraceCount=True):
    GenerateAnnotationCommon(analysis.GenerateComment, EA, Erase, PrintTraceCount)