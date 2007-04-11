import sys
import os
import urllib
import time
import ambulant

class AmbulantPlayer:
    def __init__(self):
        # Install the preferences
        pass
        # Enable the locale support
        pass
        # Load the plugins
        ##pars_f = ambulant.get_parser_factory()
        ##plugins = ambulant.get_plugin_engine()
        ##plugins.add_plugins(pars_f)
        # Initialize default systemTest settings
        pass
        # Initialize data directory
        pass        
        
class WrapWindowFactory:
    def __init__(self, wf):
        self.wf = wf
        
    def new_background_renderer(self, *args):
        print "WrapWindowFactory.new_background_renderer(self=%s, args=%s)" % (self, args)
        rv = self.wf.new_background_renderer(*args)
        print "--> %s" % rv
        return rv
    
    def new_window(self, *args):
        print "WrapWindowFactory.new_window(self=%s, args=%s)" % (self, args)
        rv = self.wf.new_window(*args)
        print "--> %s" % rv
        return rv
    
    def window_done(self, *args):
        print "WrapWindowFactory.window_done(self=%s, args=%s)" % (self, args)
        rv = self.wf.window_done(*args)
        print "--> %s" % rv
        return rv
    
class WrapPlayableFactory:
    def __init__(self, pf):
        self.pf = pf
    
    def new_playable(self, *args):
        print "WrapPlayableFactory.new_playable(self=%s, args=%s)" % (self, args)
        rv = self.pf.new_playable(*args)
        print "--> %s" % rv
        return rv
    
    def new_aux_audio_playable(self, *args):
        print "WrapPlayableFactory.new_aux_audio_playable(self=%s, args=%s)" % (self, args)
        rv = self.pf.new_aux_audio_playable(*args)
        print "--> %s" % rv
        return rv
    
#class AmbulantDocumentPlayer(ambulant.document_embedder):
class AmbulantDocumentPlayer:
    # For now, inherit the application, because we will only play
    # a single document
    
    def __init__(self, url):
        # Create the factories
        real_window_f = ambulant.none_window_factory()
        window_f = WrapWindowFactory(real_window_f)
        parser_f = ambulant.get_parser_factory()
        datasource_f = ambulant.datasource_factory()
        datasource_f.add_raw_factory(ambulant.get_ffmpeg_raw_datasource_factory())
        real_playable_f = ambulant.get_global_playable_factory()
        playable_f = WrapPlayableFactory(real_playable_f)
        self.factories = (playable_f, window_f, datasource_f, parser_f)
        self.document = None
        self.player = None
        self.opendoc(url)
        
    def opendoc(self, url):
        # Create the document
        self.document = self.create_document(url)
        if self.document is None: return
        # Create the player
        print 'Creating player'
        self.player = ambulant.create_smil2_player(self.document, self.factories, self)
        #self.player = ambulant.create_mms_player(self.document, self.factories)
        print 'Initializing player'
        #self.player.initialize()
        
    def create_document(self, url):
        print 'Reading document data'
        ok, data = ambulant.read_data_from_url(url, self.factories[2])
        if not ok:
            print 'Read_data_from_url failed'
        print 'Creating document'
        document = ambulant.create_from_string(self.factories, data, url)
        document.set_src_url(url)
        return document
        
    def play(self):
        if self.player:
            self.player.start()
        while not self.player.is_done():
            time.sleep(1)
        
    # document_embedder interface
    def show_file(self, url):
        print "python.document_embedder.show_file(%s)" % url
        
    def close(self, player):
        print "python.document_embedder.close()"
        
    def open(self, url, start, oldplayer):
        print "python.document_embedder.open(%s, %s)" % (url, start)
        
    def done(self, player):
        print "python.document_embedder.done()"
        
def main():
    if len(sys.argv) != 2:
        print "Usage: %s smilurl" % sys.argv[0]
        sys.exit(1)
    url = sys.argv[1]
    
    # Hack pathnames to urls
    if not ':' in url:
        url = os.path.abspath(url)
        url = urllib.pathname2url(url)
        if not ':' in url:
            url = 'file://' + url
    print "Opening document: %s" % url
    player = AmbulantDocumentPlayer(url)
    print "Start playback..."
    player.play()
    print "Playback done."
    
if __name__ == "__main__":
    main()
    
