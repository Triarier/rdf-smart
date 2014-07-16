#!/usr/bin/ruby
require 'minitest/autorun'
require 'minitest/spec'
require File.dirname(__FILE__) + '/../lib/rdf/smart'

describe RDF::Smart do
  it "runs a query (e.g. 'select * where {?s ?p ?o}') and returns a Hash (JSON) with the results" do
    RDF::Smart.new.execute("select * where {?s ?p ?o}").must_be_instance_of Hash
  end
end

