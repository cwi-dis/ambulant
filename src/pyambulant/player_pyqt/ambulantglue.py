import ambulant

class jaja:
    pass
    
class Glue(ambulant.gui_player, ambulant.factories):

    def __init__(self, filename, widget):
    #import pdb
    #pdb.set_trace()
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
        
    def qwidget_to_ambulant(self, widget):
        """Hack: convert QWidget to CObject (which Ambulant can handle)"""
        import sip
        widget_address = sip.unwrapinstance(widget)
        widget_vptr = sip.voidptr(widget_address)
        widget_cobj = widget_vptr.ascobject()
        return widget_cobj
    
    #
    # Initialization methods - create the various factories
    #
    
    def init_window_factory(self):
        qwa = self.qwidget_to_ambulant(self.widget)
        wf = ambulant.create_qt_window_factory_unsafe(qwa, 20, self)
        #wf = ambulant.create_none_window_factory()
        self.set_window_factory(wf)
        
    def init_playable_factory(self):
        gpf = ambulant.get_global_playable_factory()
        gpf.add_factory(ambulant.create_qt_fill_playable_factory(self))
        if hasattr(ambulant, 'create_qt_html_playable_factory'):
            gpf.add_factory(ambulant.create_qt_html_playable_factory(self))
        gpf.add_factory(ambulant.create_qt_image_playable_factory(self))
        if hasattr(ambulant, 'create_qt_smiltext_playable_factory'):
            gpf.add_factory(ambulant.create_qt_smiltext_playable_factory(self))
        gpf.add_factory(ambulant.create_qt_text_playable_factory(self))
        gpf.add_factory(ambulant.create_qt_video_playable_factory(self))
        gpf.add_factory(ambulant.create_sdl_playable_factory(self))
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
        
