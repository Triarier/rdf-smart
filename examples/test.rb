#!/usr/bin/ruby
require '../base'
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

p RDF::Smart::VERSION
