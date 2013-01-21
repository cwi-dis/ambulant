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
    logger.trace('pyamplugin_playable: initialize() called')
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return
    
    pf = DummyPlayableFactory(factories, None)
    gpf = factories.get_playable_factory()
    gpf.add_factory(pf)
    
    logger.trace('pyamplugin_playable: registered')

#
# Factory function
#
class DummyPlayableFactory(ambulant.playable_factory):
    
    RENDERER_URI = "http://www.ambulantplayer.org/component/RendererDummyPython"
    
    def __init__(self, factories, machdep):
        if DEBUG: logger.debug("pyamplugin_playable: created DummyPlayableFactory %s" % self)
        self.factories = factories
        self.machdep = machdep
        
    def supports(self, renderer_select):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayableFactory.supports(%s) URI %s" % (renderer_select, renderer_select.get_renderer_uri()))
        node_uri = renderer_select.get_renderer_uri()
        return node_uri == self.RENDERER_URI
        
    def new_playable(self, context, cookie, node, evp):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayableFactory.new_playable(%s, %s, %s, %s)" % (context, cookie, node, evp))
        return DummyPlayable(context, cookie, node, evp, self.factories, self.machdep)

    def new_aux_audio_playable(self, context, cookie, node, evp, src):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayableFactory.new_playable(%s, %s, %s, %s, %s)" % (context, cookie, node, evp, src))
        return None

#
# Playable implementation
#
class DummyPlayable(ambulant.playable):

    def __init__(self, context, cookie, node, evp, factories, machdep):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable(%s, %s, %s, %s, %s, %s)" % (context, cookie, node, evp, factories, machdep))
        self.context = context
        self.cookie = cookie
        
    def init_with_node(self, node):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.init_with_node(%s)" % node)
        self.node = node
        pass
        
    def start(self, t):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.start(%s)" % t)
        self.context.started(self.cookie, 0)
        
    def stop(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.stop()")
        self.context.stopped(self.cookie, 0)
        return True
        
    def post_stop(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.post_stop()")
        pass
        
    def pause(self, display_show):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.pause(%s)" % display_show)
        pass
        
    def resume(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.resume()")
        pass
        
    def seek(self, t):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.seek(%s)" % t)
        pass
        
    def wantclicks(self, want):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.wantclicks(%s)" % want)
        pass
        
    def preroll(self, when, where, howmuch):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.preroll(%s, %s, %s)" % (when, where, howmuch))
        pass
        
    def get_dur(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.get_dur()")
        pass
        return 0

    def get_cookie(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.get_cookie()")
        return self.cookie
        
    def get_renderer(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.get_renderer()")
        return None
        
    def get_sig(self):
        if DEBUG: logger.debug("pyamplugin_playable.DummyPlayable.get_sig()")
        return repr(self)
        
