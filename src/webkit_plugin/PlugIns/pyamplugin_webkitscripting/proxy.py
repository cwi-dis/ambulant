#!/usr/bin/env python
#
# $Source$
# $Id$
#

"""
Proxy objects for any library, that allow you to add hooks before or after
methods on a specific object.

Original code from ActiveState recipe by Martin Blais <blais@furius.ca>.
Modified by Jack Jansen to do all calls to the object in the Foundation
main thread.

This is usually overkill: only the Foundation/AppKit/WebKit/etc calls have to 
be done in the main thread. But, for now we leave it like this.
"""

#===============================================================================
# EXTERNAL DECLARATIONS
#===============================================================================

import Foundation
import types
from pprint import pformat

#===============================================================================
# PUBLIC DECLARATIONS
#===============================================================================

__all__ = ['HookProxy']

#-------------------------------------------------------------------------------
#
class MTProxyMethodWrapper(Foundation.NSObject):
    """
    Wrapper object for a method to be called.
    """

    def init( self, obj, func, name ):
        self.obj, self.func, self.name = obj, func, name
        assert obj is not None
        assert func is not None
        assert name is not None
        return self

    def __call__( self, *args, **kwds ):
        pool = Foundation.NSAutoreleasePool.alloc().init()
        rvholder = []
        self.performSelectorOnMainThread_withObject_waitUntilDone_(
                self.callinmainthread_, (args, kwds, rvholder), True)
        if len(rvholder) != 1:
            print 'MTProxyMethodWrapper.__call__: Call produced no return value: ', self.name, args, kwds
            return
        return rvholder[0]

    def callinmainthread_( self, (args, kwds, rvholder) ):
        rv = self.obj._method_call(self.name, self.func, *args, **kwds)
        rvholder.append(rv)

#-------------------------------------------------------------------------------
#
class MTHookProxy(object):
    """
    Proxy object that delegates methods and attributes that don't start with _.
    You can derive from this and add appropriate hooks where needed.
    Override _pre/_post to do something before/afer all method calls.
    Override _pre_<name>/_post_<name> to hook before/after a specific call.
    """

    def __init__( self, objname, obj ):
        self._objname, self._obj = objname, obj

    def __getattribute__( self, name ):
        """
        Return a proxy wrapper object if this is a method call.
        """
        if name.startswith('_'):
            return object.__getattribute__(self, name)
        else:
            att = getattr(self._obj, name)
            if type(att) is types.MethodType:
                return MTProxyMethodWrapper.alloc().init(self, att, name)
            else:
                return att

    def __setitem__( self, key, value ):
        """
        Delegate [] syntax.
        """
        name = '__setitem__'
        att = getattr(self._obj, name)
        pmeth = MTProxyMethodWrapper(self, att, name)
        pmeth(key, value)

    def _call_str( self, _name, *_args, **kwds ):
        """
        Returns a printable version of the call.
        This can be used for tracing.
        """
        pargs = [pformat(x) for x in _args]
        for k, v in kwds.iteritems():
            pargs.append('%s=%s' % (k, pformat(v)))
        return '%s.%s(%s)' % (self._objname, _name, ', '.join(pargs))

    def _method_call( self, _name, _func, *args, **kwds ):
        """
        This method gets called before a method is called.
        """
        # pre-call hook for all calls.
        try:
            prefunc = getattr(self, '_pre')
        except AttributeError:
            pass
        else:
            prefunc(_name, *args, **kwds)

        # pre-call hook for specific method.
        try:
            prefunc = getattr(self, '_pre_%s' % _name)
        except AttributeError:
            pass
        else:
            prefunc(*args, **kwds)

        # get real method to call and call it
        rval = _func(*args, **kwds)

        # post-call hook for specific method.
        try:
            postfunc = getattr(self, '_post_%s' % _name)
        except AttributeError:
            pass
        else:
            postfunc(*args, **kwds)

        # post-call hook for all calls.
        try:
            postfunc = getattr(self, '_post')
        except AttributeError:
            pass
        else:
            postfunc(_name, *args, **kwds)

        return rval


#===============================================================================
# TEST
#===============================================================================

def test():
    import sys

    class Foo:
        def foo( self, bli ):
            print '       (running foo -> %s)' % bli
            return 42

    class BabblingFoo(MTHookProxy):
        "Proxy for Foo."
        def _pre( self, _name, *args, **kwds ):
            print >> sys.stderr, \
                  "LOG :: %s" % self._call_str(_name, *args, **kwds)

        def _post( self, _name, *args, **kwds ):
            print 'after all'

        def _pre_foo( self, *args, **kwds ):
            print 'before foo...'

        def _post_foo( self, *args, **kwds ):
            print 'after foo...'

    f = BabblingFoo('f', Foo())
    print 'rval = %s' % f.foo(17)

    # try calling non-existing method
    try:
        f.nonexisting()
        raise RuntimeError
    except AttributeError:
        pass

if __name__ == '__main__':
    test()
