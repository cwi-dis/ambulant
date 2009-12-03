import sys
import pygtk
pygtk.require('2.0')
import gtk
import os
import ambulantglue

class MyMainWidget():
    ui = '''<ui>
		<menubar name="MenuBar">
			<menu action="File">
				<menuitem action="Open"/>
				<menuitem action="Quit"/>
			</menu>
			<menu action="Controls">
				<menuitem action="Play"/>
				<menuitem action="Stop"/>
				<menuitem action="Pause"/>
			</menu>
			<menu action="Help">
				<menuitem action="About2"/>
				<menuitem action="Welcome"/>
			</menu>
		</menubar>
		<toolbar name="ToolBar">
			<toolitem action="Open"/>
			<toolitem action="Play"/>
			<toolitem action="Stop"/>
			<toolitem action="Pause"/>
			<toolitem action="About"/>
		</toolbar>
	</ui>'''

    def __init__(self):
	# Window and framework
        self.window = gtk.Window(gtk.WINDOW_TOPLEVEL)
        self.window.connect("destroy", self.destroy)
	# Laiola: used in the self.destroy method to sinalize that the program must quit
	self.exit = False

	# setup the widget container
	self.vbox = gtk.VBox()
	self.window.add(self.vbox)

	# Set title and window size
	self.window.set_title("AmbulantPlayer version PyGTK")
	self.window.set_size_request(640, 480)

	# Setup uimanager for menu and toolbar
	self.uimanager = gtk.UIManager()

	# Add accelerator group
	self.accelgroup = self.uimanager.get_accel_group()
	self.window.add_accel_group(self.accelgroup)

	# Create an ActionGroup
	self.actiongroup = gtk.ActionGroup('actions')

	# Create actions
	self.actiongroup.add_actions(
		[
			('File', None, '_File'),
			('Open', gtk.STOCK_OPEN, 'Open...', None,
			'Open...', self.do_open),
			('Quit', gtk.STOCK_QUIT, None, None,
			'Quit', self.destroy),
			('Controls', None, '_Controls'),
			('Play', gtk.STOCK_MEDIA_PLAY, None, None,
			'Play', self.do_play),
			('Stop', gtk.STOCK_MEDIA_STOP, None, None,
			'Stop', self.do_stop),
			('Pause', gtk.STOCK_MEDIA_PAUSE, None, None,
			'Pause', self.do_pause),
			('Help', None, '_Help'),
			('About2', gtk.STOCK_ABOUT, 'About player_pygtk.py...', None,
			'About', self.do_about),
			('About', gtk.STOCK_ABOUT, 'About', None,
			'About', self.do_about),
			('Welcome', gtk.STOCK_EXECUTE, 'Play _Welcome Document', None, 
			'Play _Welcome Document', self.do_welcome)
		]
	)

	# Attach the action group
	self.uimanager.insert_action_group(self.actiongroup, 0)

	# Add a UI description
	self.uimanager.add_ui_from_string(self.ui)

        self.initmenubar()
	self.inittoolbar()
        self.glue = None
        self.update_menus(None)
        self.ambulant_widget = gtk.DrawingArea() #try1
        self.vbox.add(self.ambulant_widget) #try1
#try1	self.document_widget = gtk.VBox()
#try1	self.vbox.add(self.document_widget)

	# Show the window
	self.window.show_all()

    # All PyGTK applications need a main method - event loop
    def main(self):
	# Laiola: Trick code that enables ambulant presentations to keep refreshing the screen.
	# Very ugly but it works :)

	# Laiola: trick code that seems to work
	while self.exit == False:
            self.flush_events()

	# Laiola: It does not work properly
	#gtk.main()

    def flush_events(self):
        # Updates bounding boxes, among others.
        while gtk.events_pending():
            gtk.main_iteration() 

    # Destroy method causes appliaction to exit
    # when main window closed
    def destroy(self, widget, data=None):	
	self.exit = True
	# Laiola: It is necessary only if you use gtk.main() in self.main()
        #gtk.main_quit()

    def inittoolbar(self):
	# Create toolbar
	self.toolbar = self.uimanager.get_widget('/ToolBar')
	self.vbox.pack_start(self.toolbar, False)

    def initmenubar(self):
	self.menubar = self.uimanager.get_widget('/MenuBar')
	self.vbox.pack_start(self.menubar, False)

	self.playButton = self.uimanager.get_widget('/ToolBar/Play')
        self.stopButton = self.uimanager.get_widget('/ToolBar/Stop')
	self.pauseButton = self.uimanager.get_widget('/ToolBar/Pause')

	self.playMenu = self.uimanager.get_widget('/MenuBar/Controls/Play')
	self.stopMenu = self.uimanager.get_widget('/MenuBar/Controls/Stop')
	self.pauseMenu = self.uimanager.get_widget('/MenuBar/Controls/Pause')
        
	self.playMenu.connect('select', self.update_menus)
	self.stopMenu.connect('select', self.update_menus)
	self.pauseMenu.connect('select', self.update_menus)

	self.playButton.connect('clicked', self.update_menus)
	self.stopButton.connect('clicked', self.update_menus)
	self.pauseButton.connect('clicked', self.update_menus)
	        
    def do_open(self, b):
	chooser = gtk.FileChooserDialog(title="Open Presentation",action=gtk.FILE_CHOOSER_ACTION_OPEN,
                      buttons=(gtk.STOCK_CANCEL,gtk.RESPONSE_CANCEL,gtk.STOCK_OPEN,gtk.RESPONSE_OK))
        chooser.set_default_response(gtk.RESPONSE_OK)

	filter = gtk.FileFilter()
	filter.set_name("SMIL files")
	filter.add_pattern("*.smil")
	filter.add_pattern("*.smi")
	chooser.add_filter(filter)

	filter = gtk.FileFilter()
	filter.set_name("Any file")
	filter.add_pattern("*")
	chooser.add_filter(filter)

	response = chooser.run()
	if response == gtk.RESPONSE_OK:
    	    print chooser.get_filename(), ' selected'
	    filename = chooser.get_filename()
            if filename:
                filename = str(filename)
                self.open_document(filename)

	elif response == gtk.RESPONSE_CANCEL:
	    print 'Closed, no files selected'
	chooser.destroy()
        
    def do_play(self, b):
        if self.glue:
            self.glue.play()
	self.update_menus(None)
        
    def do_stop(self, b):
        if self.glue:
            self.glue.stop()
	self.update_menus(None)
    
    def do_pause(self, b):
        if self.glue:
            self.glue.pause()
	self.update_menus(None)
        
    def xxx(self, *args):
        print 'xxx', `args`
        
    def update_menus(self, b):
	if self.glue:
            self.playMenu.set_sensitive(self.glue.is_play_enabled())
            self.stopMenu.set_sensitive(self.glue.is_stop_enabled())
            self.pauseMenu.set_sensitive(self.glue.is_pause_enabled())

            self.playButton.set_sensitive(self.glue.is_play_enabled())
            self.stopButton.set_sensitive(self.glue.is_stop_enabled())
            self.pauseButton.set_sensitive(self.glue.is_pause_enabled())
        else:
            self.playMenu.set_sensitive(False)
            self.stopMenu.set_sensitive(False)
            self.pauseMenu.set_sensitive(False)

            self.playButton.set_sensitive(False)
            self.stopButton.set_sensitive(False)
            self.pauseButton.set_sensitive(False)
            
    def do_about(self, b):
	msgBox = gtk.MessageDialog(self.window, buttons = gtk.BUTTONS_OK, 
		message_format = "Demo application embedding Ambulant Player in a Python program.")
     	msgBox.run()
     	msgBox.destroy()
        
    def do_welcome(self, b):
        welcome = self.find_document("Extras/Welcome/Welcome.smil")
        if welcome:
            self.open_document(welcome)
        else:
	    msgBox = gtk.MessageDialog(self.window, buttons = gtk.BUTTONS_OK, 
	    	message_format = "Cannot find Welcome.smil document")
     	    msgBox.run()
     	    msgBox.destroy()
                
    def find_document(self, filename):
        """Try to locate the given file somewhere along our path"""
        curdir = os.getcwd()
        while curdir and curdir != '/':
            candidate = os.path.join(curdir, filename)
            if os.path.exists(candidate):
                return candidate
            curdir = os.path.dirname(curdir)
        return None
        
    def open_document(self, document):
        if self.find_document (document) == None:
            print "Cannot find: ", document
            return None

        if not self.glue == None:
            self.glue.terminate()
            self.ambulant_widget.destroy()

        self.ambulant_widget = gtk.DrawingArea()
#try1   self.document_widget.add(self.ambulant_widget)
        self.vbox.add(self.ambulant_widget) #try1

        self.glue = ambulantglue.Glue(document, self.ambulant_widget)
	self.update_menus(None)
        self.do_play(None)
	self.update_menus(None)

    def refresh(self):
   	if self.window:
     	    width,height = self.window.get_size() 
#try1       self.document_widget.queue_draw_area(0,0,width,height)
            self.ambulant_widget.queue_draw_area(0,0,width,height) #try1
    def expose(self, widget, event):
	self.refresh()
	return False
    
# Define the main window

if __name__ == "__main__":
    base = MyMainWidget()

    #
    # Open any document given on the command line
    #
    if len(sys.argv) > 1:
        base.open_document(sys.argv[1])

    base.main()
