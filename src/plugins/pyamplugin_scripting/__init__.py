print 'pyamplugin_scripting: imported __init__'

import pyamplugin_scripting.scripting
import ambulant

keeper_hack = []

def initialize(apiversion, factories, gui_player):
    print 'pyamplugin_scripting: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    keeper_hack.append(factories)
    keeper_hack.append(gui_player)
    
    sf = pyamplugin_scripting.scripting.MyScriptComponentFactory()
    gsf = factories.get_script_component_factory()
    if not gsf:
        gsf = ambulant.get_global_script_component_factory()
        factories.set_script_component_factory(gsf)
    gsf.add_factory(sf)
    