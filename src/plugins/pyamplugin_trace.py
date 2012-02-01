import sys
import time
if sys.platform == 'win32':
	sys.stdout = sys.stderr = open('AmbulantPlayerPythonPlugin.log', 'a')
print 'pyamplugin_state: imported __init__ at ', time.asctime()

import pyamplugin_state.state
import ambulant

keeper_hack = []

def initialize(apiversion, factories, gui_player):
    print 'pyamplugin_state: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    keeper_hack.append(factories)
    keeper_hack.append(gui_player)
    
    sf = pyamplugin_state.state.MyStateComponentFactory()
    gsf = factories.get_state_component_factory()
    if not gsf:
        gsf = ambulant.get_global_state_component_factory()
        factories.set_state_component_factory(gsf)
    gsf.add_factory(sf)
    