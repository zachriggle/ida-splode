#!/usr/bin/env python
# -*- coding: latin-1 -*-
import ida, database, query, settings

def RgbToBgr(rgb):
    bgr = 0
    bgr |= rgb & 0xff0000 >> 16;
    bgr |= rgb & 0x00ff00
    bgr |= rgb & 0x0000ff << 16
    return bgr

def SetColor(ea, rgb):
    """Helper to set color in RGB, vs IDA's default BGR"""
    # print "Setting %x to %x" % (ea, rgb)
    ida.SetColor(ea, ida.CIC_ITEM, RgbToBgr(rgb))

def GetColor(ea):
    return ida.GetColor(ea, ida.CIC_ITEM)

DidColor    = False

def ColorHit(IP, Count=-1):
    """Color a single instruction"""
    Hit      = query.GetHitInstructions({'IP':IP})
    Enhanced = query.GetEnhancedInstructions({'IP':IP})

    if Enhanced: SetColor(IP, settings.COLOR_INS_ENHANCED)
    elif Hit:    SetColor(IP, settings.COLOR_INS_EXECUTED)
    else:        SetColor(IP, settings.COLOR_INS_RESET)

def ColorHits():
    """Color all instructions which PIN ever saw, and darken those with metadata"""
    global DidColor
    print "Coloring hits, please wait..."

    #
    # Clear all colors every time
    #
    for Head in ida.Heads():
        if GetColor(Head) in (settings.COLOR_INS_EXECUTED, settings.COLOR_INS_ENHANCED):
            SetColor(Head, settings.COLOR_INS_RESET)

    #
    # Set the colors every other time
    #
    if not DidColor:
        for EA in query.GetHitInstructions():
            SetColor(EA,settings.COLOR_INS_EXECUTED)

        for EA in query.GetEnhancedInstructions():
            SetColor(EA,settings.COLOR_INS_ENHANCED)

        for EA in query.AddressesTouched().keys():
            SetColor(EA, settings.COLOR_DATA_REFERENCE)

    DidColor = not DidColor
    print "...Done coloring"

def ColorDelta():
    """Color all hits in two traces"""
    OtherDatabase = ida.AskStr('', 'Select alternate database')

