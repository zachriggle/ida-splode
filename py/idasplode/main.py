#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Entry point for the ida script.
"""

# Even though we don't need all of these, import them so that they are available
# from the IDA console


def main():
    import help, hotkeys, database, traceback
    try:
        help.PrintHelp()
        hotkeys.RegisterHotkeys()
        database.LoadDatabase( database.GetLastDatabaseName()
                        or database.AskUserSelectDatabase() )
    except:
        traceback.print_exc()

if __name__ == '__main__':
    main()