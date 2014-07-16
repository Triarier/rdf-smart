#!/usr/bin/ruby
require 'minitest/autorun'
require 'minitest/spec'
require File.dirname(__FILE__) + '/../lib/rdf/smart'

describe RDF::Smart do
  it "returns a hash with the namespaces" do
    RDF::Smart.new.must_be_instance_of RDF::Smart
  end
end

