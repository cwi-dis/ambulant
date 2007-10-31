print "STEP ONE - Python code loaded into python plugin in ambulant in Safari"
import sys
##print sys.path
import ambulant
print "STEP TWO - Ambulant version is", ambulant.get_version()
import pyamplugin_webkitscripting.state
embedder = None
def set_extra_data(idd):
	global embedder
	embedder = idd
	
keeper_hack = []

def initialize(apiversion, factories, gui_player):
    print 'pyamplugin_webkitscripting: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

    keeper_hack.append(factories)
    keeper_hack.append(gui_player)
    
    print "pyamplugin_webkitscripting: WebPlugInContainer is", embedder
    embedder.webPlugInContainerShowStatus_("AmbulantWebKitPlugin: glue loaded")
    webframe = embedder.webFrame()
    print "pyamplugin_webkitscripting: WebFrame is", webframe
##    print "Pyamplugin_webkitscripting: WebFrame version is", webframe.version
##    print "contents", dir(webframe)
##    import pdb; pdb.set_trace()
    domdocument = webframe.DOMDocument()
    #
    # Distinguish between Safari 2 and 3
    #
    if hasattr(domdocument, "evaluate_____"):
        print "pyamplugin_webkitscripting: Apparently Safari 3 (with XPath support)"
        import pyamplugin_webkitscripting.state_xpath
        state = pyamplugin_webkitscripting.state_xpath
    else:
        print "pyamplugin_webkitscripting: Apparently Safari 2 (without XPath support)"
        import pyamplugin_webkitscripting.state
        state = pyamplugin_webkitscripting.state
        
    sf = state.MyStateComponentFactory(domdocument)
    gsf = factories.get_state_component_factory()
    if not gsf:
        gsf = ambulant.get_global_state_component_factory()
        factories.set_state_component_factory(gsf)
    gsf.add_factory(sf)
    
