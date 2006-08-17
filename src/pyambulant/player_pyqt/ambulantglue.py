class Glue:
    def __init__(self, filename, widget):
        self.widget = widget
        self.filename = filename
        print "Glue(%s)", filename
        
    def play(self):
        print "Glue.play()"
        
    def stop(self):
        print "Glue.stop()"
        
    def pause(self):
        print "Glue.pause()"
        
    def is_play_enabled(self):
        return True
        
    def is_stop_enabled(self):
        return True
        
    def is_pause_enabled(self):
        return True
        
    def is_play_active(self):
        return False
        
    def is_stop_active(self):
        return True
        
    def is_pause_active(self):
        return False
        