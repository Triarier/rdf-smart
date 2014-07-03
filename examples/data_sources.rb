#!/usr/bin/ruby
require '../lib/rdf/smart'

# Initialize new RDF::Smart Object with test.ttl as data_sources
x = RDF::Smart.new('test.ttl')

# Prints data_sources -> ['test.ttl']
pp x.data_sources 
# Add new data_sources 
x.data_sources << 'mh3.ttl'

# Print again -> ['test.tt','mh3.ttl']

pp x.data_sources
