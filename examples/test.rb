require '../lib/rdf-smart'

module RDF
  module SPARQL
    class Smart
      def initialize(*x)
      end

      class Namespace
        def initialize(prefix, iri)
        end

        attr_reader :prefix
        attr_reader :iri
      end

    end  
  end
end  


s = RDF::SPARQL::Smart.new
s.source << 'test.ttl'
p s.sources # [ 'test.ttl' ]
s.source << 'http://solo.wst.univie.ac.at/test.ttl'
p s.sources # [ 'test.ttl', 'http://solo.wst.univie.ac.at/test.ttl' ]
p s.namespaces # array of namespaces


r = s.execute <<-
  PREFIX wst: <http://wst.univie.ac.at/>
  SELECT distinct ?person
  FROM <test.ttl>
  WHERE {
    ?person a wst:person.
  }
end

r.count == r.length
r.json
r.head
r.results
r.results.ordered
r.results.distinct
r.results.bindings
r.results.bindings.json

# enumerate all results (include Enumerable in class)
r.each 
r.first
r.last
