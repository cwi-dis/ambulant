import sys
import qt
import os
import ambulantglue

class MyMainWidget(qt.QWidget):
    def __init__(self, parent=None, name=None):
        qt.QWidget.__init__(self, parent, name)
        self.initmenubar()
        self.glue = None
        self.update_menus()
        
    def initmenubar(self):
        self.filemenu = qt.QPopupMenu(self)
        self.filemenu.insertItem("&Open...", self.do_open, qt.Qt.CTRL+qt.Qt.Key_O)
        self.filemenu.insertItem("&Quit", qt.qApp, qt.SLOT( "quit()" ), qt.Qt.CTRL+qt.Qt.Key_Q)
        
        self.playmenu = qt.QPopupMenu(self)
        self.playID = self.playmenu.insertItem("&Play", self.do_play)
        self.stopID = self.playmenu.insertItem("&Stop", self.do_stop)
        self.pauseID = self.playmenu.insertItem("P&ause", self.do_pause)
        self.connect(self.playmenu, qt.SIGNAL("aboutToShow()"), self.update_menus)
        
        self.helpmenu = qt.QPopupMenu(self)
        self.helpmenu.insertItem("&About %s..." % self.name(), self.do_about)
        self.helpmenu.insertItem("Play &Welcome Document", self.do_welcome)
        
        self.menubar = qt.QMenuBar(self, "MainMenu")
        self.menubar.insertItem("&File", self.filemenu)
        self.menubar.insertItem("&Play", self.playmenu)
        self.menubar.insertItem("&Help", self.helpmenu)

    def do_open(self):
        filename = qt.QFileDialog.getOpenFileName(
            ".",
            "SMIL files (*.smil *.smi);; Any file (*)",
            self,
            "Open Presentation",
            "Double-click a file to open it")
        if filename:
            filename = str(filename)
            print "file = " + filename
            self.open_document(filename)
        
    def do_play(self):
        if self.glue:
            self.glue.play()
        
        
    def do_stop(self):
        if self.glue:
            self.glue.stop()
        
    def do_pause(self):
        if self.glue:
            self.glue.pause()
        
    def xxx(self, *args):
        print 'xxx', `args`
        
    def update_menus(self):
        if self.glue:
            self.playmenu.setItemEnabled(self.playID, self.glue.is_play_enabled())
            self.playmenu.setItemEnabled(self.stopID, self.glue.is_stop_enabled())
            self.playmenu.setItemEnabled(self.pauseID, self.glue.is_pause_enabled())
            self.playmenu.setItemChecked(self.playID, self.glue.is_play_active())
            self.playmenu.setItemChecked(self.stopID, self.glue.is_stop_active())
            self.playmenu.setItemChecked(self.pauseID, self.glue.is_pause_active())
        else:
            self.playmenu.setItemEnabled(self.playID, False)
            self.playmenu.setItemEnabled(self.stopID, False)
            self.playmenu.setItemEnabled(self.pauseID, False)
            self.playmenu.setItemChecked(self.playID, False)
            self.playmenu.setItemChecked(self.stopID, True)
            self.playmenu.setItemChecked(self.pauseID, False)
            
    def do_about(self):
        qt.QMessageBox.about(self, self.name(),
            "Demo application embedding Ambulant Player in a Python program.")
        
    def do_welcome(self):
        welcome = self.find_document("Extras/Welcome/Welcome.smil")
        if welcome:
            self.open_document(welcome)
        else:
            qt.QMessageBox.information(self, self.name(),
                "Cannot find Welcome.smil document")
                
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
        self.glue = ambulantglue.Glue(document, self)
        self.do_play()
    
def main():
    #
    # Create application and main widget
    #
    myapp = qt.QApplication(sys.argv)
    mywidget = MyMainWidget(name=sys.argv[0])
    #
    # Connect them, and show the main widget
    #
    myapp.setMainWidget(mywidget)
    mywidget.show()
    #
    # Open any document given on the command line
    #
    if len(sys.argv) > 1:
        mywidget.open_document(sys.argv[1])
    #
    # Now run the mainloop until the user quits or
    # closes the main window.
    #
    rv = myapp.exec_loop()
    sys.exit(rv)
    
if __name__ == '__main__':
    main()
    
