The NewYorGeo Demo
--------------------------
The purpose of this demo is to show:
 - that the player is correctly installed
 - how content in a presentation can be scaled for various players

These presentations represent both confidence tests (showing that 
audio, text and images work) and a series of checks to make sure that
the SMIL scheduler works. As a result, not all slide have the same
duration and not all layout work is 'visually' perfect.

In particular, note that NYC-LG uses captions as transparent overlays
that are sometime partially obscured by background content, while
NYC-LG-M uses a fixed background color that itself sometime obscures
underlying content.


Versions:
--------------------------
 - NYC-SM.smil: A simple MMS slideshow.
 - NYC-LG.smil: A version with better quality graphics
 - NYC-LG-M.smil: Like NYC-LG.smil, but with background music


Suggested Run Sequence:
---------------------------
1. Run NYC-SM.smil
   This shows the basic presentation for MMS on a small device.
   
2. Run NYC-LG.smil 
   This shows the same presentation as (1), but with better imaging.

3. Run NYC-LG-M.smil 
   This shows the same presentation as (1), 
   but with better imaging and background audio.

Note that the restrictions of MMS preclude running the demos on real mobile devices.
