import sys
import os
import time
import json
import ambulant
DEBUG=False

class BandwidthCollector:
	"""Record bandwidth"""
	BIN_DURATION=0.1
	
	def __init__(self, starttime, master):
		self.starttime = starttime
		self.bw = []
		self.master = master
		self.lasttime = self.starttime
		
	def addMeasurement(self, now, bw):
		self.addCumulative(self.lasttime, now, bw)
		self.lasttime = now
		
	def addCumulative(self, start, end, bw):
		startbin = int((start-self.starttime) / self.BIN_DURATION)
		endbin = int((end-self.starttime) / self.BIN_DURATION)
		bytes_per_bin = float(bw) / ((1+endbin-startbin)*self.BIN_DURATION)
		if len(self.bw) <= endbin:
			self.bw += [0]*((endbin+1)-len(self.bw))
		for i in range(startbin, endbin+1):
			self.bw[i] += bytes_per_bin
		if bw > 0 and self.master:
			self.master.addCumulative(start, end, bw)

	def asList(self, globStart):
		rv = [(self.starttime-globStart, 0)]
		time = self.starttime
		for bps in self.bw:
			time += self.BIN_DURATION
			rv.append((time-globStart, bps))
		return rv
					
class TimeRange:
	"""Records a single continuous time range"""
	
	def __init__(self, start, descr=""):
		"""Create a timerange, given the start time"""
		self.start = start
		self.fill = None
		self.stop = None
		self.descr = descr
		self.run_type = ""
		self.bw_collectors = {}
		
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
		
	def asDict(self, globStart=0, globStop=0):
		stop = self.stop
		if stop is None:
			stop = globStop
		rv = {"start":self.start-globStart, "stop":stop-globStart , "descr" : self.descr}
		if not self.fill is None:
			rv["fill"] = self.fill - globStart
		if self.bw_collectors:
			rv["bandwidth"] = {}
			for k, v in self.bw_collectors.items():
				rv["bandwidth"][k] = v.asList(globStart)
		return rv
		
	def getBandwidthCollector(self, name, topnode):
		if self.bw_collectors.has_key(name):
			return self.bw_collectors[name]
		master = None
		if topnode:
			master = topnode.getBandwidthCollector(name, None)
		bwc = BandwidthCollector(self.start, master)
		self.bw_collectors[name] = bwc
		return bwc
					
class NodeRun(TimeRange):
	"""Records a single execution of a SMIL node"""
	
	def __init__(self, node_type, descr, start):
		TimeRange.__init__(self, start)
		self.descr = descr
		self.playables = []
		self.run_type = node_type
		
	def addPlayable(self, pl):
		self.playables.append(pl)
				
class PlayableRun(TimeRange):
	"""Records a single run of a playable"""
	def __init__(self, objid, descr, start):
		TimeRange.__init__(self, start)
		self.objid = objid
		self.descr = descr
		self.stalls = []
		self.predecessor = None
		self.run_type = "playable"
		
	def stallStart(self, start, descr=""):
		if self.stalls and self.stalls[-1].is_active():
			return
		self.stalls.append(TimeRange(start, descr))
		
	def stallStop(self, stop):
		assert self.stalls
		assert self.stalls[-1].is_active()
		self.stalls[-1].setStop(stop)
		
	def inheritStall(self):
		"""Called by the successor to inherit a stall"""
		if self.stalls and self.stalls[-1].is_active():
			self.stalls[-1].setStop(self.stop)
			return True
		return False
	
	def setPredecessor(self, plrun):
		self.predecessor = plrun
		
	def asDict(self, globStart=0, globStop=0):
		stop = self.stop
		if stop is None:
			stop = globStop
		rv = TimeRange.asDict(self, globStart, globStop)
		if self.stalls:
			stallList = []
			for stall in self.stalls:
				stallList.append(stall.asDict(globStart, globStop))
			rv['stalls'] = stallList
		return rv
		
	def isStalled(self):
		return self.stalls and self.stalls[-1].is_active()
				
class DocumentRun(TimeRange):
	"""Records a single execution of a document"""
	
	def __init__(self, start):
		TimeRange.__init__(self, start)
		self.nodes = {}
		
	def addNode(self, node_id, node_type, descr, start):
		"""Add a run for a specific node"""
		if self.nodes.has_key(node_id):
			last = self.nodes[node_id][-1]
			assert not last.is_active()
		else:
			self.nodes[node_id] = []
		new_run = NodeRun(node_type, descr, start)
		self.nodes[node_id].append(new_run)
		
	def getNodeRun(self, node_id):
		"""Get the current active run for a given node"""
		assert self.nodes.has_key(node_id)
		run = self.nodes[node_id][-1]
		assert run.is_active()
		return run
		
class Collector(DocumentRun):
	"""Collect and dump information about a document run"""
	def __init__(self, descr, start):
		DocumentRun.__init__(self, start)
		self.descr = descr
		self.curPlayable = {}
		
	def now(self):
		return time.time()
		
	def setPlayable(self, plid, plrun):
		self.curPlayable[plid] = plrun
		
	def getPlayable(self, plid):
		return self.curPlayable.get(plid, None)
		
	def dump_json(self):
		if DEBUG: print "collector returning data from", self.start, 'to', self.stop
		startTime = self.start
		stopTime = self.stop
		if stopTime is None:
			stopTime = self.now()
		objects = [{"objtype": "document", "objid":"/", "runs":[self.asDict(startTime, stopTime)]}]

		# Get the (node, [run, ...]) items sorted by first begin time
		nodes = self.nodes.items()
		nodes.sort(cmp=(lambda a, b: cmp(a[1][0].start, b[1][0].start)))
		for node, runlist in nodes:
			rundata = []
			playables = {}
			objtype = "node"
			for run in runlist:
				objtype = run.run_type
				
				# For each run, append the times to the rundata for this node
				rundata.append(run.asDict(startTime, stopTime))
				
				# For each playable corresponding to this run, append/create the playable run
				for playable in run.playables:
					if playables.has_key(playable.objid):
						playables[playable.objid].append(playable.asDict(startTime, stopTime))
					else:
						playables[playable.objid] = [playable.asDict(startTime, stopTime)]

			# Output the data for the node runs
			object = {"objtype" : objtype, "objid":node, "runs" : rundata}
			objects.append(object)
			
			# Now sort the playable runs by first start time and output them too
			playableitems = playables.items()
			playableitems.sort(cmp=(lambda a, b: cmp(a[1][0]["start"], b[1][0]["start"])))
			for playableid, playableruns in playableitems:
				stalls = []
				for run in playableruns:
					if run.has_key("stalls"):
						stalls += run["stalls"]
						del run["stalls"]
				object = {"objtype" : "playable", "objid" : node + ' ' + playableid, "runs" : playableruns}
				if stalls:
					object["stalls"] = stalls
				objects.append(object)

		# All done. Dump the data.
		return json.dumps(objects, sort_keys=True, indent=4)
	
class TracePlayerFeedback(ambulant.player_feedback):
	"""This class is the observer for events that happen during playback.
	The various methods emit output for all events, this output can then
	later be structured for display.
	"""
	def timestamp(self):
		return time.strftime("%H:%M:%S") + " EXEC  "
		
	def now(self):
		return time.time()
		
	def done(self):
		if hasattr(self.next_feedback, 'done'):
			self.next_feedback.done()
		self.next_feedback = None
		
	def __init__(self):
		self.collector = None
		self.doc_url = None
		self.next_feedback = None
		
	def __del__(self):
		if DEBUG: print 'TracePlayerFeedback deleted'

	def dump_json(self):
		if self.collector:
			return self.collector.dump_json()
		else:
			if DEBUG: print self.timestamp(), 'not generating json data: no collector available'
			return None
		
	def set_next_feedback(self, next_feedback):
		self.next_feedback = next_feedback
		
	def document_loaded(self, doc):
		if DEBUG: print self.timestamp(), 'document_loaded(%s)' % doc
		self.doc_url = doc.get_url()
		if self.next_feedback: self.next_feedback.document_loaded(doc)
		
	def document_started(self):
		if DEBUG: print self.timestamp(), 'document_started()'
		self.collector = Collector(self.doc_url, self.now())
		if self.next_feedback: self.next_feedback.document_started()

	def document_stopped(self):
		if DEBUG: print self.timestamp(), 'document_stopped()'
		self.collector.setStop(self.now())
		if self.next_feedback: self.next_feedback.document_stopped()

	def node_started(self, node):
		if DEBUG: print self.timestamp(), 'node_started(%s)' % node.get_sig()
		node_id = node.get_xpath()
		node_type = node.get_local_name()
		if node_type in ("audio", "video", "text", "smilText", "img", "animation", "ref", "area"):
			node_type = "medianode"
		node_descr = node.get_sig()
		self.collector.addNode(node_id, node_type, node_descr, self.now())
		if self.next_feedback: self.next_feedback.node_started(node)

	def node_filled(self, node):
		if DEBUG: print self.timestamp(), 'node_filled(%s)' % node.get_sig()
		node_id = node.get_xpath()
		run = self.collector.getNodeRun(node_id)
		run.setFill(self.now())
		if self.next_feedback: self.next_feedback.node_filled(node)

	def node_stopped(self, node):
		if DEBUG: print self.timestamp(), 'node_stopped(%s)' % node.get_sig()
		node_id = node.get_xpath()
		run = self.collector.getNodeRun(node_id)
		run.setStop(self.now())
		if self.next_feedback: self.next_feedback.node_stopped(node)

	def _id_for_playable(self, playable):
		sig = playable.get_sig()
		commapos = sig.find(',')
		if commapos:
			sig = sig[:commapos]
		return sig.replace('(', ' ')
		 
	def playable_started(self, playable, node, comment):
		if DEBUG: print self.timestamp(), 'playable_started(%s, %s, %s)' % (playable.get_sig(), node.get_sig(), comment)
		now = self.now()
		node_id = node.get_xpath()
		run = self.collector.getNodeRun(node_id)
		plid = self._id_for_playable(playable)
		playrun = PlayableRun(plid, playable.get_sig(), now)
		predecessor = None
		if comment == "cached":
			predecessor = self.collector.getPlayable(plid)
			assert predecessor
			assert predecessor.is_active()
			predecessor.setStop(now)
			playrun.setPredecessor(predecessor)
			# And inherit any "stalled" flag
			if predecessor.inheritStall():
				playrun.stallStart(now)
		self.collector.setPlayable(plid, playrun)
		run.addPlayable(playrun)
		if self.next_feedback: self.next_feedback.playable_started(playable, node, comment)

	def playable_stalled(self, playable, reason):
		if DEBUG: print self.timestamp(), 'playable_stalled(%s, %s)' % (playable.get_sig(), reason)
		plid = self._id_for_playable(playable)
		playrun = self.collector.getPlayable(plid)
		playrun.stallStart(self.now(), reason)
		if self.next_feedback: self.next_feedback.playable_stalled(playable, reason)

	def playable_unstalled(self, playable):
		if DEBUG: print self.timestamp(), 'playable_unstalled(%s)' % playable.get_sig()
		plid = self._id_for_playable(playable)
		playrun = self.collector.getPlayable(plid)
		if playrun.isStalled:
			playrun.stallStop(self.now())
		else:
			print self.timestamp(), "playable_unstalled(%s): ignored, playable is not stalled" % playable.get_sig()
		if self.next_feedback: self.next_feedback.playable_unstalled(playable)

	def playable_cached(self, playable):
		if DEBUG: print self.timestamp(), 'playable_cached(%s)' % playable.get_sig()
		plid = self._id_for_playable(playable)
		playrun = self.collector.getPlayable(plid)
		playrun.setFill(self.now())
		if self.next_feedback: self.next_feedback.playable_cached(playable)

	def playable_deleted(self, playable):
		if DEBUG: print self.timestamp(), 'playable_deleted(%s)' % playable.get_sig()
		plid = self._id_for_playable(playable)
		playrun = self.collector.getPlayable(plid)
		playrun.setStop(self.now())
		self.collector.setPlayable(playable, None)
		if self.next_feedback: self.next_feedback.playable_deleted(playable)

	def playable_resource(self, playable, resource, amount):
		if DEBUG: print self.timestamp(), "playable_resource(%s, %s, %f)" % (playable.get_sig(), resource, amount)
		plid = self._id_for_playable(playable)
		playrun = self.collector.getPlayable(plid)
		bwc = playrun.getBandwidthCollector(resource, self.collector)
		bwc.addMeasurement(self.now(), amount)
