#!/bin/bash	
#
# make-dmg-installer from installable source(s) and a template .dmg file containing all visuals
#
# Arguments: 
# -d file-or-directory: destination on the output disk (should pair with source, -s)
# -n name: name of the output disk ($name.dmg)
# -s file-or-directory: source to be copied into a corresponding destination on the output disk
# -t template: the (.dmg) file to be used as template for the output disk
#
# A template .dmg can be constructed as follows:
# With 'Disk Utility', select 'File->New', define the Volume name and select 'read-write'.
# (or create copy from an existing template im 'Terminal:
# hdiutil convert $diskname-template.dmg -format UDRW -o $diskname-rw.dmg}.
# A disk background image can be defined by creating and copying an initial background image
# in /Volumes/$diskname/.background/background.png.
# Next, in 'Terminal' the command 'open  /Volumes/$diskname/.background' and using the 'Finder'
# window '$diskname' select 'View->Show View Option'. Then, in the 'View Options' window
# under 'Background' select 'Image' and drag file 'backgound.png' from the '.background' window
# into the provided space.
# Finally, 'Eject' the disk and compress the corresponding .dmg was by the command:
# 'hdiutil convert $diskname-rw.dmg -format UDZO -o $diskname.dmg'
# The template has all visuals (icons and background), but the files are empty.
# Their contents are copied in by executing this script.
# All content files are easily modifyable and under version control.
# To change the disk layout, repeat the disk creation procedure described above.
#
# This method seems to be more reliable than fiddling appearance using 'osascript', because some
# 'Finder' versions seem to use "optimized" techniques to handle folder backgrounds, etc. Based on:
# http://stackoverflow.com/questions/96882/how-do-i-create-a-nice-looking-dmg-for-mac-os-x-using-command-line-tools, 8th answer

# set -x

function _log() {
	if [ $log -eq 1 ] ; then echo $@; fi
}
function usage () {
	_log "usage " $@
	echo "Usage: " "-n  name -d [destination ]... [ -s source_dir ] [ -t template-dmg ]"
	if [ $# -gt 0 ] ;then echo "Error: " $1; exit -1; fi
}

# script variables

source_idx=0
dest_idx=0
declare -a source=()0
declare -a dest=()
template=
name=
debug=0
log=0

function eval_args() {
	_log "eval_args" $@

	if [ $# -eq 0 ] ; then usage "Missing argument" ; fi
	
	while [ $# -gt 1 ] ;do
		if [ $# -le 1 -a $1 != "-l" -a $1 != "-x" ] ; then usage "Missing argument" ; fi
		case $1 in
			-d) dest[$dest_idx]=$2 ; let dest_idx=$dest_idx+1 ;;
			-n) name=$2 ;;
			-t) template=$2 ;;
			-s) source[$source_idx]=$2 ; let source_idx=$source_idx+1 ;;
			# undocumented debugging options
			-x) if [ $debug -eq 0 ] ; then set -x; debug=1; else set +x; debug=0; fi; shift; continue ;;
			-l) if [ $log -eq 0 ] ; then log=1; else log=0; fi; shift; continue ;;
			-*) usage "Unknown Argument" ;;
		esac
		shift;shift
	done
}

# copy src dst - copy src to dst, recursively. src and dst are pathnames (e.g. ./file)
function copy() {
	_log "copy" $@
	if [ $# -ne 2 ] ;then usage "internal-copy"; fi
	_src="$1"
	_dst="$2"
	_src_dir=`dirname "$1"`
	_dst_dir=`dirname "$2"`
	cp -r "$_src" "$_dst"
	unset _dst _dst_dir _src _src_dir
}

# create_writable_disk template dest - create writable disk with name $dest-rw.dmg from $template
function create_writable_disk() {
	_log "create_writable_disk" $@
	_template="$1"
	_name="$2"
	rm -f "$_name.dmg"  "$_name-rw.dmg "
	cp "$_template" "$_name.dmg" 
	#
	# Convert it into a writable image and attach it to the file system (mount)
	#
	hdiutil convert "$_name.dmg" -format UDRW -o "$_name-rw.dmg"
	hdiutil attach "$_name-rw.dmg"
	unset _template _name
}

# copy_files - array copy: all files in $source is copied into /Volumes/$dest
# 	uses: arrays dest, source
function copy_files() {
	_log "copy_files" $@
	# check if source array is 1-1 mappable to dest array
	if [ ${#dest[@]} -ne ${#source[@]} ] ;then usage "internal-copy_files: no mapping"; fi
	
	idx=0
	while [ $idx -ne $source_idx ] ; do
		copy "${source[$idx]}" "/Volumes/$name/${dest[$idx]}"
		let idx=$idx+1
	done
}

function compress_disk() {
	_name="$1"
	_log "compress_disk" $@
	hdiutil detach "/Volumes/$_name"
	rm -f "$_name.dmg"
	sleep 5
	hdiutil convert "$_name-rw.dmg" -format UDZO -o "$_name.dmg"
	unset _name
}

function cleanup() {
	_log "cleanup" $@
	rm -fr "$@"
	unset source_idx dest_idx source dest template name debug log
}

# script starts

# evaluate initial -l (log) before anything else
if [ $# -ge 1 ] ;then if [ $1 = "-l" ] ;then log=1; shift; fi; fi

if [ $log -eq 1 ] ;then echo "script-args: " $# $@; fi

# evaluate all command-line arguments
eval_args "$@"

# create and  mount writable disk named $name-rw.dmg from $template 
create_writable_disk "$template" "$name"

# Copy all source files on the writable disk
copy_files

# compress and finalize the newly created disk
compress_disk "$name"

if [ $log -gt 0 ] ; then
	echo "name="$name
	echo "template="$template
	echo "source($source_idx): "${source[@]}
	echo "dest($dest_idx): "${dest[@]}
fi

# cleanup temp. files
cleanup "$name-rw.dmg"
