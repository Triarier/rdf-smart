#!/usr/bin/ruby
require File.dirname(__FILE__) + '/../lib/rdf/smart'

# Initialize new RDF::Smart Object without any data_sources
x = RDF::Smart.new()

# Prints Version of RDF-Smart
pp x.version
