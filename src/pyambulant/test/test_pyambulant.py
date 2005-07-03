# Test suite for pyambulant.
import ambulant
import unittest
import time

VERSION="1.5" # Ambulant version

class TestBasics(unittest.TestCase):

    def setUp(self):
        pass
        
    def test01getversion(self):
        version = ambulant.get_version()
        self.assertEqual(version, VERSION)
        
    def test02realtimetimer(self):
        rtt = ambulant.realtime_timer_factory()
        self.assert_(type(rtt) is ambulant.abstract_timer)
        t1 = rtt.elapsed()
        self.assert_(t1 > 0)
        time.sleep(1)
        t2 = rtt.elapsed()
        self.assertAlmostEqual(t1+1000, t2, -1)
        
    def test03eventprocessor(self):
        rtt = ambulant.realtime_timer_factory()
        evp = ambulant.event_processor_factory(rtt)
        self.assert_(type(evp) is ambulant.event_processor)
        time.sleep(1)
        t1 = rtt.elapsed()
        evp_timer = evp.get_timer_1()
        t2 = evp_timer.elapsed()
        self.assertAlmostEqual(t1, t2, -1)
        
    def test04document(self):
        rf = ambulant.get_global_playable_factory()
        wf = ambulant.none_window_factory()
        df = ambulant.datasource_factory()
        pf = ambulant.get_parser_factory()
        factories = (rf, wf, df, pf)
        doc = ambulant.create_from_string(factories, "<smil></smil>", "file:///test.smil")
        self.assert_(type(doc) is ambulant.document)
        root = doc.get_root()
        self.assert_(type(root) is ambulant.node)
        
if __name__ == "__main__":
    unittest.main()
    
