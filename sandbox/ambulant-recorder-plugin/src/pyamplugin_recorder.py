import ambulant
import os
import subprocess
import threading
import Queue

DEBUG=False
# with THREADED=True, the writing of data to the pipe is done in a seperate thread
THREADED=True

#
# Trickery for logging output: if we cannot get at the Ambulant
# logger we use a dummmy implementation that prints to stdout.
#
class DummyLogger:
    def trace(self, str):
        print str
    
    debug = error = warn = show = trace
    
logger = ambulant.get_logger_1("")
if not logger:
    logger = DummyLogger()

#
# Main entry point
#
def initialize(apiversion, factories, gui_player):
    logger.trace('pyamplugin_recorder: initialize() called')
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return
    
    rf = DummyRecorderFactory(factories, None)
    factories.set_recorder_factory(rf)
    
    logger.trace('pyamplugin_recorder: registered')

#
# Factory function
#
class DummyRecorderFactory(ambulant.recorder_factory):
    
    def __init__(self, factories, machdep):
        if DEBUG: logger.debug("pyamplugin_recorder: created DummyRecorderFactory %s" % self)
        self.factories = factories
        self.machdep = machdep
        
    def new_recorder(self, pixel_order, size):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorderFactory.new_recorder(%s, %s)" % (pixel_order, size))
        return DummyRecorder(pixel_order, size)

    def new_aux_audio_recorder(self, context, cookie, node, evp, src):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorderFactory.new_recorder(%s, %s, %s, %s, %s)" % (context, cookie, node, evp, src))
        return None

#
# Recorder implementation
#
class DummyRecorder(ambulant.recorder):

    def __init__(self, pixel_order, size):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder(%s, %s)" % (pixel_order, size))
        self.pixel_order = pixel_order
        self.size = size
        ambulant_recorder_pipe = os.getenv("AMBULANT_RECORDER_PIPE")
        print ambulant_recorder_pipe
        if ambulant_recorder_pipe == None:
            self.pipe = None
        else:
            self.pipe = os.popen2(ambulant_recorder_pipe)[0]
        if THREADED: # create queue and worker thread
            self.queue = Queue.Queue();
            self.alive = True;
            self.thread = threading.Thread(target=self.worker)
            self.thread.setDaemon(True)
            self.thread.start()

    def new_video_data(self, data, timestamp):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder.new_video_data: size=%d timestamp=%d)" % (len(data), timestamp))
        if THREADED:
            self.queue.put((timestamp, data))
        else:
            self.write_header(timestamp, len(data))
            self.write_data(data)
        pass

    def new_audio_data(self, data, timestamp):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder.new_audio_data %s %s)" % (data, timestamp))
        pass
# write fixed size (80 bytes) header preceding variable size pixel data        
    def write_header(self, timestamp, datasize):
        s = "Time: %08lu\nSize: %08lu\nW: %#5u\nH: %#5u\nChksm: %024lx\n" % (timestamp, datasize, self.size[0], self.size[1], 0)
#       s = "%#8lu\n" % timestamp
#       print "pyamplugin_recorder.DummyRecorder.write_header() s=%s" % s
#       print "pipe=%r " % self.pipe
        if self.pipe != None:
            self.pipe.write(s)
        pass

    def write_data(self, data):
        if self.pipe != None:
            self.pipe.write(data)
            self.pipe.flush()
        pass

    def worker(self):
        while self.alive:
            (timestamp, data) = self.queue.get()
            if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder.worker: size=%d timestamp=%d)" % (len(data), timestamp))
            self.write_header(timestamp, len(data))
            self.write_data(data)
#           self.queue.task_done()