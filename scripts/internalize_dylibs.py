#
# Examine a framework and recursively search for external
# dynamic libraries. Copy all of these into the framework.
#
import sys
import os
import shutil
import subprocess
import re
import getopt

OTOOL_MATCHER=re.compile(r'(.*) \(compatibility version .*, current version .*\)')

MACOSX_BUNDLE_DIRS = (
	'Contents/MacOS',
	'Contents/Frameworks',
	'Contents/PlugIns',
	[
		'/System/',
		'/lib/',
		'/usr/lib/',
		'/usr/X11/lib/',
		'@loader_path/',
		'@executable_path/',
	]
)


# XXX Needs work for iPhone (because of different relative paths)

class Internalizer:
	def __init__(self, bundle, (run_dir, framework_dir, plugin_dir, safe_prefixes)):
		self.bundle_dir = bundle
		self.run_dir = os.path.join(bundle, run_dir)
		self.framework_dir = os.path.join(bundle, framework_dir)
		self.plugin_dir = os.path.join(bundle, plugin_dir)
		self.safe_prefixes = safe_prefixes
		self.safe_prefixes.append(bundle) # XXX good idea?
		
		self.dstroot = None
		if os.environ.has_key('DSTROOT'):
			self.dstroot = os.environ['DSTROOT']
		
		self.todo = {}
		self.done = {}
		self.used = {}
				
		self.norun = False
		self.verbose = False
		self.work_done = False
		
		self.instlibdir = None
		self.staginglibdir = None
		
		self.errors_encountered = False
		
	def dstrelative(self, path):
		if self.dstroot and os.path.isabs(path):
			newpath = self.dstroot + path
			if os.path.exists(newpath):
				return newpath
		return path
		
	def set_staging_libdir(self, instlibdir, staginglibdir):
		self.instlibdir = instlibdir
		self.staginglibdir = staginglibdir
		
	def add_standard(self):
		for dirpath, dirnames, filenames in os.walk(self.run_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		for dirpath, dirnames, filenames in os.walk(self.plugin_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)
		for dirpath, dirnames, filenames in os.walk(self.framework_dir):
			for name in filenames:
				name = os.path.join(dirpath, name)
				self.add(name)

	def add(self, src, copy=False):
	    # See if we should copy from staging dir in stead of installdir
		if self.instlibdir and os.path.commonprefix([src, self.instlibdir]) == self.instlibdir:
			better_src = self.staginglibdir + src[len(self.instlibdir):]
			if os.path.exists(self.dstrelative(better_src)):
			    src = better_src
		while os.path.islink(self.dstrelative(src)):
			src = os.path.realpath(self.dstrelative(src))
		if src in self.todo or src in self.done:
			return
		if not os.path.exists(self.dstrelative(src)):
			print '** file does not exist:', src
			self.errors_encountered = True
			self.work_done = True
		if not self.is_loadable(self.dstrelative(src)):
			return
		if self.verbose: print '* add', src
		if copy:
			dstname = os.path.basename(src)
			if dstname in self.used:
				print '** destname', dstname, 'in use'
				# Is this an error? Unsure... self.errors_encountered = True
				return
			self.used[dstname] = True
		else:
			dstname = None
		self.todo[src] = dstname
		
	def run(self):
		while self.todo:
			src, dst = self.todo.items()[0]
			self.process(self.dstrelative(src), dst)
			del self.todo[src]
			self.done[src] = dst
		
	def process(self, src, dst):
		if self.verbose: print '* process', src, dst
		libraries = self.get_libs(src)
		must_change = []
		for lib in libraries:
			if self.must_copy(lib):
				self.add(lib, copy=True)
				must_change.append(lib)
		if dst:
			self.copy(src, dst)
			self.set_name(dst)
			src = os.path.join(self.framework_dir, dst)
		for lib in must_change:
			self.modify_reference(src, lib)
		
	def copy(self, src, dst):
		if '.framework/' in src:
			print '** Warning: About to copy from framework:', src
		dstfilename = os.path.join(self.framework_dir, dst)
		if self.verbose:
			print 'copy', src, dstfilename
		if not self.norun:
			dstdir = os.path.dirname(dstfilename)
			if not os.path.exists(dstdir):
				os.mkdir(dstdir)
			shutil.copy(src, dstfilename)
		self.work_done = True
		
	def get_libs(self, src):
		proc = subprocess.Popen(['otool', '-L', src],
			stdout=subprocess.PIPE)
		rv = []
		for line in proc.stdout.readlines():
			line = line.strip()
			matches = OTOOL_MATCHER.match(line)
			if matches:
				rv.append(matches.group(1))
		return rv
		
	def set_name(self, dst):
		dstfilename = os.path.join(self.framework_dir, dst)
		dstfileid = os.path.join("@loader_path/../Frameworks/", os.path.basename(dst))
		if self.verbose:
			print 'setname', dstfilename, dstfileid
		if not self.norun:
			subprocess.check_call(['install_name_tool', '-id', dstfileid, dstfilename])
		self.work_done = True
			
	def modify_reference(self, dst, lib):
		reallib = os.path.realpath(lib)
		libid = os.path.join("@loader_path/../Frameworks/", os.path.basename(reallib))
		if self.verbose:
			print 'modify_lib_reference', dst, lib, libid
		if not self.norun:
			subprocess.check_call(['install_name_tool', '-change', lib, libid, dst])
		if not self.verbose and self.norun:
			print 'Warning: %s has reference to %s' % (dst, lib)
		self.work_done = True
			
	def must_copy(self, lib):
		for prefix in self.safe_prefixes:
			if  os.path.commonprefix([prefix, lib]) == prefix:
				return False
		return True
		
	def is_loadable(self, file):
		proc = subprocess.Popen(['file', file], 
				stdout=subprocess.PIPE,
				stderr=open('/dev/null', 'w'))
		line = proc.stdout.read()
		rv = ('Mach-O executable' in line or
			'Mach-O 64-bit executable' in line or
			'Mach-O bundle' in line or
			'Mach-O 64-bit bundle' in line or
			'Mach-O dynamically linked shared library' in line or
			'Mach-O 64-bit dynamically linked shared library' in line)
		sts = proc.wait()
		return rv
		
def main():
	norun = False
	verbose = False
	check = False
	instlibdir = None
	reallibdir = None
	try:
		opts, args = getopt.getopt(sys.argv[1:], 'vncs:')
		for o, v in opts:
			if o == '-v':	
				verbose = True
			if o == '-n':
				norun = True
			if o == '-c':
				check = True
			if o == '-s':
				if not ':' in v:
					raise getopt.error
				instlibdir, reallibdir = v.split(':')
		if len(args) != 1:
			raise getopt.error('missing arguments')
	except getopt.error:
		print 'Usage: %s [-vnc] [-s instlibdir:reallibdir] bundlepath '% sys.argv[0]
		print 'Recursively slurp dylibs used in a bundle.'
		print '-n\tNo-run, only print actions, do not do the work'
		print '-v\tVerbose, print actions as well as doing them'
		print '-c\tCheck, do nothing, print nothing, return nonzero exit status if there was work'
		print '-s\tSet library directory substitution (for uninstalled libraries)'
		sys.exit(1)
	internalizer = Internalizer(os.path.realpath(args[0]), MACOSX_BUNDLE_DIRS)
	if norun:
		internalizer.norun = True
		internalizer.verbose = True
	elif verbose:
		internalizer.verbose = True
	elif check:
		internalizer.norun = True
	if instlibdir:
		internalizer.set_staging_libdir(instlibdir, reallibdir)
	internalizer.add_standard()
	internalizer.run()
	
	if internalizer.errors_encountered:
	    print 'Errors encountered during internalization'
	    sys.exit(1)
	if check:
		if internalizer.work_done:
			sys.exit(1)
		
		
if __name__ == '__main__':
	main()
	
