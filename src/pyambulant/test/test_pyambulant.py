# Test suite for pyambulant.
import ambulant
import unittest

VERSION="1.5" # Ambulant version

class TestBasics(unittest.TestCase):

    def setUp(self):
        pass
        
    def test1getversion(self):
        version = ambulant.get_version()
        self.assertEqual(version, VERSION)
        
if __name__ == "__main__":
    unittest.main()
    
