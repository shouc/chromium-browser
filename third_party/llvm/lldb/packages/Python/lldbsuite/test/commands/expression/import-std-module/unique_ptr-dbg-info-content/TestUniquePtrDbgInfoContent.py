"""
Test std::unique_ptr functionality with a decl from debug info as content.
"""

from lldbsuite.test.decorators import *
from lldbsuite.test.lldbtest import *
from lldbsuite.test import lldbutil

class TestUniquePtrDbgInfoContent(TestBase):

    mydir = TestBase.compute_mydir(__file__)

    @add_test_categories(["libc++"])
    @skipIf(compiler=no_match("clang"))
    def test(self):
        self.build()

        lldbutil.run_to_source_breakpoint(self,
            "// Set break point at this line.", lldb.SBFileSpec("main.cpp"))

        self.runCmd("settings set target.import-std-module true")

        self.expect("expr (int)s->a", substrs=['(int) $0 = 3'])
        self.expect("expr (int)(s->a = 5)", substrs=['(int) $1 = 5'])
        self.expect("expr (int)s->a", substrs=['(int) $2 = 5'])
        self.expect("expr (bool)s", substrs=['(bool) $3 = true'])
        self.expect("expr s.reset()")
        self.expect("expr (bool)s", substrs=['(bool) $4 = false'])

