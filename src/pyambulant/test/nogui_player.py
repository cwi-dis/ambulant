import sys
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
        
        
#class AmbulantDocumentPlayer(ambulant.document_embedder):
class AmbulantDocumentPlayer:
    # For now, inherit the application, because we will only play
    # a single document
    
    def __init__(self, url):
        # Create the factories
        window_f = ambulant.none_window_factory()
        parser_f = ambulant.get_parser_factory()
        datasource_f = ambulant.datasource_factory()
        playable_f = ambulant.get_global_playable_factory()
        self.factories = (window_f, parser_f, datasource_f, playable_f)
        self.document = None
        self.player = None
        self.opendoc(url)
        
    def opendoc(self, url):
        # Create the document
        self.document = self.create_document(url)
        
        # Create the player
        self.player = create_smil2_player(self.document, self.factories, self)
        self.player.initialize()
        
    def create_document(self, url):
        data = ambulant.read_data_from_url(url, self.factories[2])
        document = ambulant.create_from_string(self.factories, data, url)
        document.set_src_url(url)
        return document
        
    def play(self):
        self.player.play()
        
    # document_embedder interface
    def show_file(self, url):
        print "python.document_embedder.show_file(%s)" % url
        
    def close(self, player):
        print "python.document_embedder.close()"
        
    def open(self, url, start, oldplayer):
        print "python.document_embedder.open(%s, %s)" % (url, start)
        
def main():
    if len(sys.argv) != 2:
        print "Usage: %s smilurl" % sys.argv[0]
        sys.exit(1)
    print "Opening document..."
    player = AmbulantDocumentPlayer(sys.argv[1])
    print "Start playback..."
    player.play()
    print "Playback done."
    
if __name__ == "__main__":
    main()
    
