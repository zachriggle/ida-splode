import multiprocessing as mp
import pymongo, os, time
mongoworkerCount = int(0.75*mp.cpu_count())

class MongoWorker(mp.Process):
    def __init__(self, queue, db, coll):
        super(MongoWorker, self).__init__()
        self.queue = queue
        self.db    = db
        self.coll  = coll

    def run(self):
        client = pymongo.MongoClient()
        db     = client[self.db]
        c      = db[self.coll]
        bulk   = []
        for data in iter( self.queue.get, None ):
            c.insert(eval(data))

class FileWorker(mp.Process):
    def __init__(self, queue, filename):
        super(FileWorker, self).__init__()
        self.queue    = queue
        self.filename = filename
    def run(self):
        totalbytes = os.path.getsize(self.filename)
        iterations = 0
        MiB        = 1024*1024
        q = self.queue
        for data in file(self.filename):
            q.put( data )
            iterations += 1
            totalbytes -= len(data)
            if iterations % 1000 == 0:
                print "\r%s %i MiB remaining" % (self.filename, totalbytes / MiB),
        for i in range(mongoworkerCount):
            q.put( None )

def ProcessFileIntoCollectition(filename, databaseName, collectionName):
    pymongo.MongoClient()[databaseName].drop_collection(collectionName)
    request_queue = mp.Queue(mongoworkerCount*100)
    workers = []
    for i in range(mongoworkerCount):
        workers.append(MongoWorker( request_queue, databaseName, collectionName))
    workers.append(FileWorker( request_queue, filename ))
    map(mp.Process.start, workers)
    map(mp.Process.join, workers)
    print "\n%s Done" % filename