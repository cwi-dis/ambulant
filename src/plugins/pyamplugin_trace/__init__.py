import tracer
import webserver
import ambulant
DEBUG=True


class TraceEmbedder(ambulant.embedder):
	"""Helper class. We must insert our TracePlayerFeedback into the player
	object, but this player object has not been created yet at the time the
	initialize() call is made. Therefore we have to extend the embedder object
	to be able to intercept the starting() callback, which is emitted when 
	the player object has been created.
	"""
	def __init__(self, old_embedder, feedback, server):
		self.old_embedder = old_embedder
		self.feedback = feedback
		self.server = server
		
	def __del__(self):
		if DEBUG: print 'TraceEmbedder deleted'
		
	def close(self, player):
		if DEBUG: print 'TraceEmbedder.close()'
		return self.old_embedder.close(player)
		
	def open(self, newdoc, start, oldplayer):
		return self.old_embedder.open(newdoc, start, oldplayer)
		
	def done(self, player):
		if DEBUG: print 'TraceEmbedder.done()'
		self.feedback.done()
		self.server.stop()
		self.feedback = None
		self.server = None
		rv = self.old_embedder.done(player)
		self.old_embedder = None
		return rv
		
	def starting(self, player):
		old_feedback = player.get_feedback()
		self.feedback.set_next_feedback(old_feedback)
		player.set_feedback(self.feedback)
		self.server.start()
		return self.old_embedder.starting(player)
		
	def aux_open(self, url):
		return self.old_embedder.aux_open(url)
		
def initialize(apiversion, factories, gui_player):
	"""Called every time the plugin is loaded. This happens both at application
	startup time and at document startup time"""
	
	if not gui_player:
		# This is the initial initialize call, before a document
		# is opened. Ignore, we'll get another one later.
		print 'pyamplugin_trace: initialize() called without document'
		return

	print 'pyamplugin_trace: initialize() called for document'
	# Create the trace object, the web server  and the embedder helper object that
	# will insert the trace object into the player (once it is initialized)
	collector = tracer.TracePlayerFeedback()
	server = webserver.WebServer()
	server.setTracer(collector)
	old_embedder = gui_player.get_embedder()
	embedder = TraceEmbedder(old_embedder, collector, server)
	gui_player.set_embedder(embedder)
