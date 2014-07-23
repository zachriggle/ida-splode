#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Entry point for the ida script.
"""

# Even though we don't need all of these, import them so that they are available
# from the IDA console


def main():
    from idasplode import help, hotkeys, database
    try:
        help.PrintHelp()
        hotkeys.RegisterHotkeys()
        database.LoadDatabase( database.GetLastDatabaseName()
                        or database.AskUserSelectDatabase() )
    except:
        import traceback
        traceback.print_exc()
    del help, hotkeys, database

if __name__ == '__main__':
    main()