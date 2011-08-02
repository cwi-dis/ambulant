#
# Generate a HTML filethat references to a CAB file. This allows
# Internet Explorer users to install the plugin.
#
import sys

TEMPLATE="""<html>
<title>Install Ambulant Plugin for Explorer</title>
<h1>Install Ambulant Plugin for Explorer</h1>
<h2>version %(version)s</h2>

<p>
If you are visiting this page with Internet Explorer, you should get a message
about downloading the Ambulant plugin. Once you have done that, in the space below
this paragraph you should see the Ambulant Welcome presentation play.
</p>

<object classid="CLSID:014B38CC-E346-4813-AB87-026677D4C75D" 
  codebase="%(url)s" 
  src="http://www.ambulantplayer.org/Demos/Welcome/Welcome-smiltext.smil"
  type="application/x-ambulant-smil" 
  width="240" 
  height="270" 
  id="my_ambulant_plugin">
<param name="src" value="http://www.ambulantplayer.org/Demos/Welcome/Welcome-smiltext.smil">
<!-- We hide the inner stuff from IE through magic conditional code -->
<!--[if !IE]><![IGNORE[--><![IGNORE[]]>
  <!-- The inner object tag is for all other browsers. NOTE: use same ID as for outer tag with _inner appended -->
  <p>It seems you are using another browser than Internet Explorer. This page is not for you.
  </p>
 <!--<![endif]-->
</object>
</html>

"""

if len(sys.argv) != 3:
    print 'Usage %s version url' % sys.argv[0]
    sys.exit(1)
vars = {'version':sys.argv[1], 'url':sys.argv[2]}
data = TEMPLATE % vars
sys.stdout.write(data)

