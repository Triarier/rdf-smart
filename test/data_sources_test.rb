#!/usr/bin/ruby
require 'minitest/autorun'
require 'minitest/spec'
require File.dirname(__FILE__) + '/../lib/rdf/smart'

describe RDF::Smart do
  it "can be created with no arguments" do
    RDF::Smart.new.data_sources.must_be_instance_of Array
  end
  it "can be created with an arbitarliy number of strings" do
    RDF::Smart.new().data_sources.size.must_equal 0
    RDF::Smart.new("foo").data_sources.size.must_equal 1
    RDF::Smart.new("foo","bar").data_sources.size.must_equal 2
    RDF::Smart.new("foo","bar","foo").data_sources.size.must_equal 3
    RDF::Smart.new("foo","bar","foo","bar").data_sources.size.must_equal 4
  end
  it "accepts << for more data_sources" do
    0.upto 20 do |i| 
      foo = RDF::Smart.new()
      i.times { foo.data_sources << "foo" }
      foo.data_sources.size.must_equal i
    end
  end
end
