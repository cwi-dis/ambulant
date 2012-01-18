import sys
import time
if sys.platform == 'win32':
	sys.stdout = sys.stderr = open('AmbulantPlayerPythonPlugin.log', 'a')
print 'pyamplugin_timesync: imported __init__ at ', time.asctime()

import ambulant

DEBUG=True

keeper_hack = []

class MyTimerSyncFactory(ambulant.timer_sync_factory):
	def __init__(self):
		pass
		
	def new_timer_sync(self, document):
		return MyTimerSync(document)
		
class MyTimerSync(ambulant.timer_sync):
	def __init__(self, doc):
		if DEBUG: print 'pyamplugin_timeync(%s): __init__(%s)' % (self, doc)
		self.doc = doc
		self.timer = None
		
	def __del__(self):
		if DEBUG: print 'pyamplugin_timeync(%s): __del__()' % (self)
		if self.timer:
			self.timer.set_observer(None)
		
	def initialize(self, timer):
		if DEBUG: print 'pyamplugin_timeync(%s): initialize(%s)' % (self, timer)
		self.timer = timer
		self.timer.set_observer(self)
		
	def started(self):
		if DEBUG: print 'pyamplugin_timeync(%s): started()' % (self)
		
	def stopped(self):
		if DEBUG: print 'pyamplugin_timeync(%s): stopped()' % (self)
		
	def paused(self):
		if DEBUG: print 'pyamplugin_timeync(%s): paused()' % (self)
		
	def resumed(self):
		if DEBUG: print 'pyamplugin_timeync(%s): resumed()' % (self)
		
	
def initialize(apiversion, factories, gui_player):
    print 'pyamplugin_timesync: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    keeper_hack.append(factories)
    keeper_hack.append(gui_player)
    
    sf = MyTimerSyncFactory()
    keeper_hack.append(sf)
    keeper_hack.append(sf)
    gsf = factories.get_timer_sync_factory()
    if  gsf:
    	print 'pyamplugin_timesync: overriding older timer_sync_factory'
    factories.set_timer_sync_factory(sf)
    print 'pyamplugin_timersync: registered'
    
