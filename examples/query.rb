#!/usr/bin/ruby
require File.dirname(__FILE__) + '/../lib/rdf/smart'
require 'pp'

# Initialize new RDF::Smart Object with test.ttl as data_sources
x = RDF::Smart.new('test.ttl')

# Executes a query and returns a JSON Object. Printing JSON object. 
# The query "select * where { ?s ?p ?o}" will return all tripplets

query = "select * where {?s ?p ?o}"
pp x.execute(query)

# Add new data_sources 
x.data_sources << 'mh3.ttl'

# Use the query on all data_sources

pp x.execute(query)

pp x.data_sources
