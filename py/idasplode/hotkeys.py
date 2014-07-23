#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Routines and help information about ida_splode hotkeys
"""

import ida, color, database, analysis, help, traceback, comment, query


HotkeyBanner = """
HOTKEYS

    CTRL SHIFT H        [H]elp message
    CTRL SHIFT C        [C]olor instruction hits (repeat => clear)
    CTRL SHIFT D        [D]etails for instruction (repeat => clear)
    CTRL SHIFT E-Q      R[E]construct type from VALUE read/written
    CTRL SHIFT E-A      R[E]construct type from ADDRESS read/written
    CTRL SHIFT F        Color Di[F]ference Between Databases
    CTRL SHIFT L        [L]oad Database by Name
    CTRL SHIFT N        [N]ext traced instruction
    CTRL SHIFT R        [R]aw data for instruction
    CTRL SHIFT S        [S]ummary for Instruction (repeat => clear)
    CTRL SHIFT X        [X]ref creation for whole module
    CTRL SHIFT U        D[U]mp traces for the selected addresses
    CTRL SHIFT P        Hea[P] allocation traces/info

"""

HotkeysAreRegistered = False

def WithTryExcept(routine):
    try:    routine()
    except: traceback.print_exc()

def RegisterHotkeys():
    global HotkeysAreRegistered
    if not HotkeysAreRegistered:
        ida.add_hotkey('Ctrl-Shift-H', lambda: WithTryExcept(help.PrintHelp))
        ida.add_hotkey('Ctrl-Shift-C', lambda: WithTryExcept(color.ColorHits))
        ida.add_hotkey('Ctrl-Shift-D', lambda: WithTryExcept(comment.GenerateDetailedAnnotation))
        ida.add_hotkey('Ctrl-Shift-L', lambda: WithTryExcept(database.LoadDatabase))
        ida.add_hotkey('Ctrl-Shift-N', lambda: WithTryExcept(analysis.GoToNext))
        ida.add_hotkey('Ctrl-Shift-R', lambda: WithTryExcept(analysis.RawData))
        ida.add_hotkey('Ctrl-Shift-S', lambda: WithTryExcept(comment.GenerateSummaryAnnotation))
        ida.add_hotkey('Ctrl-Shift-E-A', lambda: WithTryExcept(analysis.ReconstructTypeAddr))
        ida.add_hotkey('Ctrl-Shift-E-Q', lambda: WithTryExcept(analysis.ReconstructTypeValue))
        ida.add_hotkey('Ctrl-Shift-V', lambda: WithTryExcept(analysis.MakeVtableStructs))
        ida.add_hotkey('Ctrl-Shift-X', lambda: WithTryExcept(analysis.MarkDynamicXrefs))
        ida.add_hotkey('Ctrl-Shift-U', lambda: WithTryExcept(query.DumpAddress))
        ida.add_hotkey('Ctrl-Shift-P', lambda: WithTryExcept(analysis.DumpHeapAllocations))
        HotkeysAreRegistered = True

def ListHotkeys():
    print HotkeyBanner





