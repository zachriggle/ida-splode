#!/usr/bin/env python
# -*- coding: latin-1 -*-
"""
Contains routines for manipulating the MongoDB database.
"""

import  time, ida, pymongo, addr, settings

db      = None
traces  = None
modules = None

try:
    c = pymongo.MongoClient()
except:
    print "Could not connect to MongoDB!"
    # raise

def OnFirstLoad():
    AutoLoad = GetLastDatabaseName()
    if settings.DATABASE_AUTO_LOAD_MOST_RECENT and AutoLoad in DatabaseNames():
        LoadDatabase(AutoLoad, refresh=True)

def ListDatabases():
    """
    Prints the instructional banner, reloads the available databases,
    and automatically loads the most-recently-loaded database.
    """
    Databases = DatabaseNames()

    if Databases:
        print "==== Matching Databases ===="
        for name in reversed(Databases):
            print "- " + name
        print "\n(Queried at %s)" % time.asctime()


def DropAll():
    """Drops all databases on the Mongo server"""
    for db in c.database_names():
        c.drop_database(db)

def DatabaseNames():
    """
    Generates a list of database names which are valid for the file
    currently loaded in IDA
    """
    InputMD5  = ida.GetInputFileMD5()
    Databases = c.database_names()
    ValidDBs  = []

    for Database in Databases:
        try:
            if c[Database].modules.find_one({'MD5':InputMD5}):
                ValidDBs.append(str(Database))
        except:
            pass

    return ValidDBs

def SetLastDatabaseName(name):
    """
    Saves the provided database name as the most recently used;
    this value will be returned by future calls to GetLastDatabaseName
    """
    node = ida.netnode('$ ida_splode', 0, 1)
    node.set(str(name))

def GetLastDatabaseName():
    """Returns:
        Name of the most recent database loaded by the user
    """
    node = ida.netnode('$ ida_splode', 0, 1)
    return node.valstr()

def AskUserSelectDatabase(default=None):
    """Prompts the user to enter a databse name, with an optional default.
    If no default is specified, the first database returned by MongoDB is
    used as a default.

    Returns:
        String name of database the user entered
    """
    DbNames = DatabaseNames()
    if not len(DbNames):
        print "There are no databases containing a file with MD5 %s" % ida.GetInputFileMD5()
    if default is None:
        default  = '' if not DbNames else DbNames[0]
    return ida.AskStr(default,'Select a database by name')

def LoadDatabase(name=None, refresh=True):
    """Loads a specific database.  Queries the user if no name is specified."""
    global db
    global traces
    global modules

    if name is None:
        name = AskUserSelectDatabase()
    if name not in DatabaseNames():
        err = "Selected database %r does not exist, or does not contain %s with MD5 %s"
        print err % (name, ida.GetInputFile(), ida.GetInputFileMD5())

    if name in DatabaseNames():
        SetLastDatabaseName(name)
        db           = c[name]
        traces       = db.traces
        modules      = db.modules

        traces.ensure_index('IP')

        RebaseIdaToMatchDatabase()
        # if refresh:
            # comment.RefreshAnnotations()
        # help.PrintHelp()

        print "Loaded %s" % (name)

def RebaseIdaToMatchDatabase():
    """
    Rebase the file in IDA so that the in-module addresses line up
    with the traces.
    """
    ida.Wait()
    InputMD5    = ida.GetInputFileMD5()
    Module      = modules.find_one({'MD5':InputMD5})

    if not Module:
        raise Exception("Could not find module MD5 of input module: %s" % InputMD5)

    EA          = ida.ScreenEA()
    NewBase     = Module['Base']
    OldBase     = addr.Min() & ~0xffff
    if OldBase != NewBase:
        ida.rebase_program(NewBase - OldBase, 0)
        ida.Jump(EA - OldBase + NewBase)
    ida.Wait()