import ambulant
import gobject

class jaja:
    pass
    
class Glue(ambulant.gui_player, ambulant.factories):

    def __init__(self, filename, widget):
        ambulant.gui_player.__init__(self)
        self.widget = widget

        #
        # Initialize the gui_player infrastructure
        #
        self.init_factories()
        # XXX The init_*_factory methods should have been called
        # by init_factories, but they aren't???
        self.init_window_factory()
        self.init_playable_factory()
        self.init_datasource_factory()
        self.init_parser_factory()
        self.init_plugins()
        
        #
        # Parse the document, create and initialize the player
        #
        self.document = ambulant.create_from_file(self, filename)
        player = ambulant.create_smil2_player(self.document, self, None)
        player.initialize()
        self.set_player(player)

    def terminate(self):
        self.get_player().terminate()
        self.set_player(None) # triggers release()

    def gtkwidget_to_ambulant(self, widget):
        """Nothing necessary to do here"""
        return widget
    
    #
    # Initialization methods - create the various factories
    #
    
    def init_window_factory(self):
        gwa = self.gtkwidget_to_ambulant(self.widget)
        wf = ambulant.create_gtk_window_factory_unsafe(gwa, self)
        self.set_window_factory(wf)
        
    def init_playable_factory(self):
        gpf = ambulant.get_global_playable_factory()
        gpf.add_factory(ambulant.create_gtk_fill_playable_factory(self))
        if hasattr(ambulant, 'create_gtk_html_playable_factory'):
            gpf.add_factory(ambulant.create_gtk_html_playable_factory(self))
        gpf.add_factory(ambulant.create_gtk_image_playable_factory(self))
        gpf.add_factory(ambulant.create_gtk_smiltext_playable_factory(self))
        gpf.add_factory(ambulant.create_gtk_text_playable_factory(self))
        gpf.add_factory(ambulant.create_gtk_video_playable_factory(self))
        if hasattr(ambulant, 'create_sdl_playable_factory'):
            gpf.add_factory(ambulant.create_sdl_playable_factory(self))
        if hasattr(ambulant, 'create_gstreamer_playable_factory'):
            gpf.add_factory(ambulant.create_gstreamer_playable_factory(self))
        self.set_playable_factory(gpf)
        
    def init_datasource_factory(self):
        gdf = ambulant.datasource_factory()
        #gdf.add_raw_factory(ambulant.create_posix_datasource_factory())
        gdf.add_raw_factory(ambulant.get_ffmpeg_raw_datasource_factory())
        gdf.add_video_factory(ambulant.get_ffmpeg_video_datasource_factory())
        gdf.add_audio_factory(ambulant.get_ffmpeg_audio_datasource_factory())
        #gdf.add_audio_parser_finder(get_ffmpeg_audio_parser_finder())
        gdf.add_audio_filter_finder(ambulant.get_ffmpeg_audio_filter_finder())
        self.set_datasource_factory(gdf)
        
    def init_parser_factory(self):
        pf = ambulant.get_parser_factory()
        self.set_parser_factory(pf)
        
