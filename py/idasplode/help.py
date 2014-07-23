#!/usr/bin/env python
# -*- coding: latin-1 -*-

import database, hotkeys

def PrintHelp():
    print banner
    hotkeys.ListHotkeys()
    database.ListDatabases()
    
banner = """
//******************************************************************************
//                                  IDA SPLODE
//******************************************************************************
"""
