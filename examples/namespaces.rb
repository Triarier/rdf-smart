#!/usr/bin/ruby
require File.dirname(__FILE__) + '/../lib/rdf/smart'

# Initialize new RDF::Smart Object with mh3.ttl as data_source
x = RDF::Smart.new('mh3.ttl')

# Prints all defined namespaces in the ttl-File. 
pp x.namespaces
