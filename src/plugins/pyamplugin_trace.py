import sys
import time
import ambulant
DEBUG=True

class TracePlayerFeedback(ambulant.player_feedback):
    """This class is the observer for events that happen during playback.
    The various methods emit output for all events, this output can then
    later be structured for display.
    """
    def timestamp(self):
        return time.strftime("%H:%M:%S") + " EXEC  "
        
    def __init__(self):
        pass
        
    def document_loaded(self, doc):
        if DEBUG: print self.timestamp(), 'document_loaded(%s)' % doc
        
    def document_started(self):
        if DEBUG: print self.timestamp(), 'document_started()'

    def document_stopped(self):
        if DEBUG: print self.timestamp(), 'document_stopped()'

    def node_started(self, node):
        if DEBUG: print self.timestamp(), 'node_started(%s)' % node.get_sig()

    def node_stopped(self, node):
        if DEBUG: print self.timestamp(), 'node_stopped(%s)' % node.get_sig()

    def node_focussed(self, node):
        if DEBUG:
            if node:
                print self.timestamp(), 'node_focussed(%s)' % node.get_sig()
            else:
                print self.timestamp(), 'node_focussed(None)'

    def playable_started(self, playable, node, from_cache, is_prefetch):
        if DEBUG: print self.timestamp(), 'playable_started(%s, %s, %s, %s)' % (playable.get_sig(), node.get_sig(), from_cache, is_prefetch)

    def playable_seek(self, playable):
        if DEBUG: print self.timestamp(), 'playable_seek(%s)' % playable.get_sig()

    def playable_cached(self, playable):
        if DEBUG: print self.timestamp(), 'playable_cached(%s)' % playable.get_sig()

    def playable_deleted(self, playable):
        if DEBUG: print self.timestamp(), 'playable_deleted(%s)' % playable.get_sig()

class TraceEmbedder(ambulant.embedder):
    """Helper class. We must insert our TracePlayerFeedback into the player
    object, but this player object has not been created yet at the time the
    initialize() call is made. Therefore we have to extend the embedder object
    to be able to intercept the starting() callback, which is emitted when 
    the player object has been created.
    """
    def __init__(self, old_embedder, feedback):
        self.old_embedder = old_embedder
        self.feedback = feedback
        
    def close(self, player):
        return self.old_embedder.close(player)
        
    def open(self, newdoc, start, oldplayer):
        return self.old_embedder.open(newdoc, start, oldplayer)
        
    def done(self, player):
        return self.old_embedder.done(player)
        
    def starting(self, player):
        player.set_feedback(self.feedback)
        return self.old_embedder.starting(player)
        
    def aux_open(self, url):
        return self.old_embedder.aux_open(url)

def initialize(apiversion, factories, gui_player):
    """Called every time the plugin is loaded. This happens both at application
    startup time and at document startup time"""
    
    print 'pyamplugin_trace: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    # Create the trace object and the embedder helper object that
    # will insert the trace object into the player (once it is initialized)
    tracer = TracePlayerFeedback()
    old_embedder = gui_player.get_embedder()
    embedder = TraceEmbedder(old_embedder, tracer)
    gui_player.set_embedder(embedder)
