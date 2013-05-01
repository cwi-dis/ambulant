import ambulant

DEBUG=True

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
    
    pf = DummyRecorderFactory(factories, None)
    factories.set_recorder_factory(pf)
    
    logger.trace('pyamplugin_recorder: registered')

#
# Factory function
#
class DummyRecorderFactory(ambulant.recorder_factory):
    
    def __init__(self, factories, machdep):
        if DEBUG: logger.debug("pyamplugin_recorder: created DummyRecorderFactory %s" % self)
        self.factories = factories
        self.machdep = machdep
        
    def new_recorder(self, context, cookie, node, evp):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorderFactory.new_recorder(%s, %s, %s, %s)" % (context, cookie, node, evp))
        return DummyRecorder(context, cookie, node, evp, self.factories, self.machdep)

    def new_aux_audio_recorder(self, context, cookie, node, evp, src):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorderFactory.new_recorder(%s, %s, %s, %s, %s)" % (context, cookie, node, evp, src))
        return None

#
# Recorder implementation
#
class DummyRecorder(ambulant.recorder):

    def __init__(self, context, cookie, node, evp, factories, machdep):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder(%s, %s, %s, %s, %s, %s)" % (context, cookie, node, evp, factories, machdep))
        self.context = context
        self.cookie = cookie
        
    def new_video_data(self, data, datasize, timestamp):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder.new_video_data%s %s %s)" % (data, datasize, timestamp))
        pass

    def new_audio_data(self, data, datasize, timestamp):
        if DEBUG: logger.debug("pyamplugin_recorder.DummyRecorder.new_audio_data%s %s %s)" % (data, datasize, timestamp))
        pass
        
