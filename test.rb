#!/usr/bin/ruby
require './smart'
require 'json'

module RDF
  class Smart
    def execute(query)
      data_sources.each do |s|
        raise 'File not found!' unless File.exists?(s)
      end
      JSON::parse(__execute(query))
    end
  end
end  

a = RDF::Smart.new('test.ttl')
p a.namespaces
