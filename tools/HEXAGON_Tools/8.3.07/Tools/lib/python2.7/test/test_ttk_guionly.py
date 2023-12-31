import os
import unittest
from test import test_support

# Skip this test if _tkinter wasn't built or gui resource is not available.
test_support.import_module('_tkinter')
test_support.requires('gui')

this_dir = os.path.dirname(os.path.abspath(__file__))
lib_tk_test = os.path.abspath(os.path.join(this_dir, os.path.pardir,
    'lib-tk', 'test'))

with test_support.DirsOnSysPath(lib_tk_test):
    import runtktests

import ttk
from _tkinter import TclError

try:
    ttk.Button()
except TclError, msg:
    # assuming ttk is not available
    raise unittest.SkipTest("ttk not available: %s" % msg)

def test_main():
    with test_support.DirsOnSysPath(lib_tk_test):
        from test_ttk.support import get_tk_root
        try:
            test_support.run_unittest(
                *runtktests.get_tests(text=False, packages=['test_ttk']))
        finally:
            get_tk_root().destroy()

if __name__ == '__main__':
    test_main()
