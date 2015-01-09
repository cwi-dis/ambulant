# Test suite for pyambulant.
import ambulant
import unittest
import time
import os
import urllib

VERSION="2.6" # Ambulant version
WELCOME="../../Extras/Welcome/Welcome.smil"
DOCUMENT="""<?xml version="1.0"?>
<!DOCTYPE smil PUBLIC "-//W3C//DTD SMIL 2.0//EN"
                      "http://www.w3.org/2001/SMIL20/SMIL20.dtd">
<smil xmlns="http://www.w3.org/2001/SMIL20/Language">
  <head>
  </head>
  <body>
    <par id="par1">
      <img id="img1" src="img1.gif" dur="2s"/>
      <par id="par2">
        <img id="img2" src="img2.gif" dur="1s"/>
        <img id="img3" src="img3.gif" dur="3s"/>
      </par>
    </par>
  </body>
</smil>
"""
class TestBasics(unittest.TestCase):

    def setUp(self):
        pass
        
    def _getfactories(self):
        rf = ambulant.get_global_playable_factory()
        wf = ambulant.none_window_factory()
        df = ambulant.datasource_factory()
        pf = ambulant.get_parser_factory()
        nf = ambulant.get_builtin_node_factory()
        factories = ambulant.factories()
        factories.set_playable_factory(rf)
        factories.set_window_factory(wf)
        factories.set_datasource_factory(df)
        factories.set_parser_factory(pf)
        factories.set_node_factory(nf)
        return factories
        
    def test_01_getversion(self):
        version = ambulant.get_version()
        self.assertEqual(version, VERSION)
        
    def test_02_realtimetimer(self):
        rtt = ambulant.realtime_timer_factory()
        self.assert_(type(rtt) is ambulant.timer)
        t1 = rtt.elapsed()
        self.assert_(t1 > 0)
        time.sleep(1)
        t2 = rtt.elapsed()
        self.assertAlmostEqual(t1+1000, t2, -21)
        
    def test_03_eventprocessor(self):
        rtt = ambulant.realtime_timer_factory()
        evp = ambulant.event_processor_factory(rtt)
        self.assert_(type(evp) is ambulant.event_processor)
        time.sleep(1)
        t1 = rtt.elapsed()
        evp_timer = evp.get_timer()
        t2 = evp_timer.elapsed()
        self.assertAlmostEqual(t1, t2, -1)
        
    def x_test_031_eventprocessorrttsubclass(self):

        class MyTimerWrapper(ambulant.timer):
            def __init__(self, rtt):
                self.rtt = rtt
                
            def elapsed(self):
                return self.rtt.elapsed()*2
                
        rtt_real = ambulant.realtime_timer_factory()
        rtt = MyTimerWrapper(rtt_real)
        evp = ambulant.event_processor_factory(rtt)
        self.assert_(type(evp) is ambulant.event_processor)
        time.sleep(1)
        t1 = rtt_real.elapsed()
        evp_timer = evp.get_timer()
        t2 = evp_timer.elapsed()
        self.assertAlmostEqual(t1*2, t2, -1)
        del evp
        
    def test_04_document(self):
        factories = self._getfactories()
        doc = ambulant.create_from_string(factories, "<smil></smil>", "file:///test.smil")
        self.assert_(type(doc) is ambulant.document)
        root = doc.get_root()
        self.assert_(type(root) is ambulant.node)
        self.assertEqual(root.size(), 1)
        
    def test_05_baddocument(self):
        factories = self._getfactories()
        doc = ambulant.create_from_string(factories, "<smil></smil>", "file:///test.smil")
        self.assert_(type(doc) is ambulant.document)
        root = doc.get_root()
        self.assert_(type(root) is ambulant.node)
        self.assertEqual(root.size(), 1)
        
    def test_06_node(self):
        factories = self._getfactories()
        doc = ambulant.create_from_string(factories, DOCUMENT, "file:///testdir/test.smil")
        self.assert_(doc)
        root = doc.get_root()
        self.assertEqual(root.get_local_name(), "smil")
        self.assertEqual(root.size(), 8)
        p1 = doc.get_node("par1")
        self.assertEqual(p1.get_local_name(), "par")
        self.assertEqual(p1.get_attribute_1("id"), "par1")
        self.assertEqual(p1.get_attribute_2("id"), "par1")

        self.assertEqual(p1.get_root(), root)
        self.assertNotEqual(p1, root)

        p1_path = p1.get_xpath()
        self.assertEqual(p1_path, "/smil/body/par")
        self.assertEqual(root.locate_node("body/par"), p1)

    def test_07_node_factory(self):
        class MyNodeFactory(ambulant.node_factory):
            def __init__(self):
                pass
            def new_node(self, tag, arglist, context):
                orig_nf = ambulant.get_builtin_node_factory()
                return orig_nf.new_node_1(tag, arglist, context)

        factories = self._getfactories()
        factories.set_node_factory(MyNodeFactory())
        doc = ambulant.create_from_string(factories, DOCUMENT, "file:///testdir/test.smil")
        self.assert_(doc)
        root = doc.get_root()
        self.assertEqual(root.get_local_name(), "smil")
        self.assertEqual(root.size(), 8)
        p1 = doc.get_node("par1")
        self.assertEqual(p1.get_local_name(), "par")
        self.assertEqual(p1.get_attribute_1("id"), "par1")
        self.assertEqual(p1.get_attribute_2("id"), "par1")

        self.assertEqual(p1.get_root(), root)
        self.assertNotEqual(p1, root)

        p1_path = p1.get_xpath()
        self.assertEqual(p1_path, "/smil/body/par")
        self.assertEqual(root.locate_node("body/par"), p1)
        
    def x_test_07_mmsplayer(self):
        factories = self._getfactories()
        doc = ambulant.create_from_file(factories, WELCOME)
        self.assert_(doc)
        player = ambulant.create_mms_player(doc, factories)
        # self.assert_(not player.is_playing())
        player.start()
        player.stop()
       
    def x_test_08_smil2player(self):
        class MyEmbedder(ambulant.embedder):
            pass
        factories = self._getfactories()
        doc = ambulant.create_from_file(factories, WELCOME)
        self.assert_(doc)
        embedder = MyEmbedder()
        player = ambulant.create_smil2_player(doc, factories, embedder)
        # self.assert_(not player.is_playing())
        player.start()
        time.sleep(5)
        player.stop()
        
    def test_09_datasource(self):
        if hasattr(ambulant, 'get_posix_datasource_factory'):
            rdf = ambulant.get_posix_datasource_factory()
        elif hasattr(ambulant, 'get_stdio_datasource_factory'):
            rdf = ambulant.get_stdio_datasource_factory()
        else:
            rdf = ambulant.get_ffmpeg_raw_datasource_factory()
        df = ambulant.datasource_factory()
        df.add_raw_factory(rdf)
        welcome_path = os.path.abspath(os.path.join(os.getcwd(), WELCOME))
        welcome_url = urllib.pathname2url(welcome_path)
        ok, data = ambulant.read_data_from_url(welcome_url, df)
        self.assert_(ok)
        data_orig = open(WELCOME, 'rb').read()
        self.assertEqual(len(data), len(data_orig))
        self.assertEqual(data, data_orig)
       
def test_main():
    suite = unittest.TestSuite()
    suite.addTest(unittest.makeSuite(TestBasics))
    unittest.TextTestRunner(verbosity=2).run(suite)

        
if __name__ == "__main__":
    test_main()
    
