import os

if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "STEP ONE - Python code loaded into python plugin in ambulant in Safari"
import sys
##print sys.path
import ambulant
if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "STEP TWO - Ambulant version is", ambulant.get_version()
import pyamplugin_webkitscripting.state

embedder = None
def set_extra_data(idd):
	global embedder
	if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "STEP THREE - embedder is", idd
	embedder = idd
	
keeper_hack = []

##import warnings
##warnings.filterwarnings("error")

def initialize(apiversion, factories, gui_player):
    if os.getenv("AMBULANT_PLUGIN_DEBUG"): print 'pyamplugin_webkitscripting: initialize() called'
    if not gui_player:
    	# This is the initial initialize call, before a document
    	# is opened. Ignore, we'll get another one later.
    	return

#    keeper_hack.append(factories)
#    keeper_hack.append(gui_player)
    
    if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "pyamplugin_webkitscripting: WebPlugInContainer is", embedder
    embedder.webPlugInContainerShowStatus_("AmbulantWebKitPlugin: glue loaded")
    webframe = embedder.webFrame()
    if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "pyamplugin_webkitscripting: WebFrame is", webframe
##    print "Pyamplugin_webkitscripting: WebFrame version is", webframe.version
##    print "contents", dir(webframe)
##    import pdb; pdb.set_trace()
    domdocument = webframe.DOMDocument()

    #
    # Distinguish between Safari 2 and 3
    #
    if hasattr(domdocument, "evaluate_____"):
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "pyamplugin_webkitscripting: Apparently Safari 3 (with XPath support), assume FormFaces"
        webview = webframe.webView()
        scriptobject = webview.windowScriptObject()
        # Check to see whether FormFaces is already initialized (at which point it adds an "xform" global variable.
        # We need to do this because FormFaces actually re-generates the whole webpage and reloads it. We do not want
        # to initialize our world on the first pass (when there's nothing there, and moreover this world is
        # going to disappear shortly).
        xform = scriptobject.evaluateWebScript_("xform")
        if not xform:
            print "pyamplugin_webkitscripting: FormFaces not initialized yet, bailing out"
            return
        import pyamplugin_webkitscripting.state_xpath
        state = pyamplugin_webkitscripting.state_xpath
        sf = state.MyFormFacesStateComponentFactory(domdocument, scriptobject)
        print "pyamplugin_webkitscripting: loaded FormFaces-compatible SMILState implementation"
    elif 0:  # Safari 3 with another XForms implementation
        if os.getenv("AMBULANT_PLUGIN_DEBUG"): print "pyamplugin_webkitscripting: Apparently Safari 3 (with XPath support)"
        import pyamplugin_webkitscripting.state_xpath
        state = pyamplugin_webkitscripting.state_xpath
        sf = state.MyStateComponentFactory(domdocument)
        print "pyamplugin_webkitscripting: loaded XPath-compatible SMILState implementation"
    else:
        print "pyamplugin_webkitscripting: Not Safari >= 3.0, no compatible SMILState implementation available."
       
    gsf = factories.get_state_component_factory()
    if not gsf:
        gsf = ambulant.get_global_state_component_factory()
        factories.set_state_component_factory(gsf)
    gsf.add_factory(sf)
    
