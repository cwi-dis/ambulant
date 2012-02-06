import sys
import os
import time
import json
import ambulant
DEBUG=True

class TimeRange:
	"""Records a single continuous time range"""
	
	def __init__(self, start):
		"""Create a timerange, given the start time"""
		self.start = start
		self.fill = None
		self.stop = None
		self.descr = ""
		
	def setFill(self, fill):
		"""Set the time that fill begain"""
		assert self.stop is None
		if self.fill is None:
			self.fill = fill
		
	def setStop(self, stop):
		"""Finalise a timerange by giving the stop time"""
		assert self.stop is None
		self.stop = stop
		
	def is_active(self):
		"""Return true if the timerange has not been finished yet"""
		return self.stop is None
		
	def asDict(self, globStart=0):
		rv = { "start":self.start-globStart, "stop":self.stop-globStart , "descr" : self.descr}
		if not self.fill is None:
			rv["fill"] = self.fill - globStart
		return rv
		
class NodeRun(TimeRange):
	"""Records a single execution of a SMIL node"""
	
	def __init__(self, descr, start):
		TimeRange.__init__(self, start)
		self.descr = descr
		self.playables = []
				
class PlayableRun(TimeRange):
	"""Records a single run of a playable"""
	def __init__(self, descr, start):
		TimeRange.__init__(self, start)
		self.descr = descr
		self.stalls = []
		
	def stallStart(self, start):
		if self.stalls and self.stalls[-1].is_active():
			return
		self.stalls.append(TimeRange(start))
		
	def stallStop(self, stop):
		assert self.stalls
		assert self.stalls[-1].is_active()
		self.stalls[-1].setStop(stop)
	
class DocumentRun(TimeRange):
	"""Records a single execution of a document"""
	
	def __init__(self, start):
		TimeRange.__init__(self, start)
		self.nodes = {}
		
	def addNode(self, node_id, descr, start):
		"""Add a run for a specific node"""
		if self.nodes.has_key(node_id):
			last = self.nodes[node_id][-1]
			assert not last.is_active()
		else:
			self.nodes[node_id] = []
		new_run = NodeRun(descr, start)
		self.nodes[node_id].append(new_run)
		
	def getNodeRun(self, node_id):
		"""Get the current active run for a given node"""
		assert self.nodes.has_key(node_id)
		run = self.nodes[node_id][-1]
		assert run.is_active()
		return run
		
class Collector(DocumentRun):
	"""Collect and dump information about a document run"""
	def __init__(self, filename, descr, start):
		DocumentRun.__init__(self, start)
		self.filename = filename
		self.descr = descr
		
	def setStop(self, stop):
		DocumentRun.setStop(self, stop)
		# End-of-document implies end-of-node for all nodes (this may not
		# have triggered if the stop was because of the user pressing stop).
		for runs in self.nodes.values():
			if runs[-1].is_active():
				runs[-1].setStop(stop)
		self.dump()
		
	def dump(self):
		if os.path.splitext(self.filename)[1] == '.json':
			self.dump_json()
		else:
			self.dump_csv()
			
	def dump_csv(self):
		if DEBUG: print 'dumping data to', self.filename
		fp = open(self.filename, 'w')
		
		# Dump the header and the document data
		fp.write('object, description, start, stop\n')
		fp.write('"/", "%s", %f, %f\n' % (self.descr, 0, self.stop - self.start))
		
		# Get the (node, [run, ...]) items sorted by first begin time
		nodes = self.nodes.items()
		nodes.sort(cmp=(lambda a, b: cmp(a[1][0].start, b[1][0].start)))
		for node, runlist in nodes:

			# Get the description. If the node has different descriptions
			# (because it is using smil-state-based urls, for example) we
			# want to record this fact
			descrlist = map(lambda a: a.descr, runlist)
			descrlist.sort()
			if descrlist[0] == descrlist[-1]:
				descr = descrlist[0]
			else:
				descr = "multiple values"
			
			# Dump the data
			fp.write('"%s", "%s"' % (node, descr))
			for run in runlist:
				fp.write(', %f, %f' % (run.start-self.start, run.stop-self.start))
			fp.write('\n')

	def dump_json(self):
		if DEBUG: print 'dumping data to', self.filename
		fp = open(self.filename, 'w')
		objects = [{"object":"/", "runs":[self.asDict(globStart=self.start)]}]

		# Get the (node, [run, ...]) items sorted by first begin time
		nodes = self.nodes.items()
		nodes.sort(cmp=(lambda a, b: cmp(a[1][0].start, b[1][0].start)))
		for node, runlist in nodes:
			rundata = []
			for run in runlist:
				rundata.append(run.asDict(globStart=self.start))
			object = {"object":node, "runs" : rundata}
			objects.append(object)
		fp.write(json.dumps(objects, sort_keys=True, indent=4))
	
class TracePlayerFeedback(ambulant.player_feedback):
    """This class is the observer for events that happen during playback.
    The various methods emit output for all events, this output can then
    later be structured for display.
    """
    def timestamp(self):
        return time.strftime("%H:%M:%S") + " EXEC  "
        
    def now(self):
    	return time.time()
    	
    def __init__(self):
        self.collector = None
        self.doc_url = None
        
    def document_loaded(self, doc):
        if DEBUG: print self.timestamp(), 'document_loaded(%s)' % doc
        self.doc_url = doc.get_url()
        
    def document_started(self):
        if DEBUG: print self.timestamp(), 'document_started()'
        self.collector = Collector('/tmp/smilrun.json', self.doc_url, self.now())

    def document_stopped(self):
        if DEBUG: print self.timestamp(), 'document_stopped()'
        self.collector.setStop(self.now())

    def node_started(self, node):
        if DEBUG: print self.timestamp(), 'node_started(%s)' % node.get_sig()
        node_id = node.get_xpath()
        node_descr = node.get_sig()
        self.collector.addNode(node_id, node_descr, self.now())

    def node_filled(self, node):
        if DEBUG: print self.timestamp(), 'node_filled(%s)' % node.get_sig()
        node_id = node.get_xpath()
        run = self.collector.getNodeRun(node_id)
        run.setFill(self.now())

    def node_stopped(self, node):
        if DEBUG: print self.timestamp(), 'node_stopped(%s)' % node.get_sig()
        node_id = node.get_xpath()
        run = self.collector.getNodeRun(node_id)
        run.setStop(self.now())

    def playable_started(self, playable, node, from_cache, is_prefetch):
        if DEBUG: print self.timestamp(), 'playable_started(%s, %s, %s, %s)' % (playable.get_sig(), node.get_sig(), from_cache, is_prefetch)

    def playable_stalled(self, playable, reason):
        if DEBUG: print self.timestamp(), 'playable_stalled(%s, %s)' % (playable.get_sig(), reason)

    def playable_unstalled(self, playable):
        if DEBUG: print self.timestamp(), 'playable_unstalled(%s)' % playable.get_sig()

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
