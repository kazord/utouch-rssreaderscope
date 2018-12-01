#!/usr/bin/env python3

from scope_harness import (
    CategoryListMatcher, CategoryListMatcherMode, CategoryMatcher,
    Parameters, ResultMatcher, ScopeHarness
)
from scope_harness.testing import *
import unittest, sys, os
from subprocess import Popen, PIPE

class AppsTest (ScopeHarnessTestCase):
    def start_harness(self):
        self.harness = ScopeHarness.new_from_scope_list(Parameters([SCOPE_INI]))
        self.view = self.harness.results_view
        self.view.active_scope = SCOPE_NAME


    def test_surfacing_results(self):
        self.start_harness()
        self.view.search_query = ''

        match = CategoryListMatcher() \
            .has_exactly(1) \
            .mode(CategoryListMatcherMode.BY_ID) \
            .category(CategoryMatcher("results") \
                .title("2 results") \
                .has_at_least(2) \
                .result(ResultMatcher("uri") \
                    .title("default") \
                    .art("art.png") \
                    .subtitle("subtitle") ) \
                .result(ResultMatcher("uri2") \
                    .title("default") \
                    .art("art2.png") \
                    .subtitle("subtitle2") ) ) \
            .match(self.view.categories)
        self.assertMatchResult(match)


    def test_search_results(self):
        self.start_harness()
        self.view.search_query = 'test'

        match = CategoryListMatcher() \
            .has_exactly(1) \
            .mode(CategoryListMatcherMode.BY_ID) \
            .category(CategoryMatcher("results") \
                .title("2 results") \
                .has_at_least(2) \
                .result(ResultMatcher("uri") \
                    .title("test") \
                    .art("art.png") \
                    .subtitle("subtitle") ) \
                .result(ResultMatcher("uri2") \
                    .title("test") \
                    .art("art2.png") \
                    .subtitle("subtitle2") ) ) \
            .match(self.view.categories)
        self.assertMatchResult(match)


if __name__ == '__main__':
    SCOPE_NAME = sys.argv[1]
    SCOPE_INI = sys.argv[2]

    unittest.main(argv = sys.argv[:1])


