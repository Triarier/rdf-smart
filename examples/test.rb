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


s = RDF::SPARQL::Smart.new <<-
  PREFIX wst: <http://wst.univie.ac.at/>
  SELECT distinct ?person
  FROM <test.ttl>
  WHERE {
    ?person a wst:person.
  }
end
p s.data_sources # [ 'test.ttl' ]
s.data_sources << 'http://solo.wst.univie.ac.at/test.ttl'
p s.data_sources # [ 'test.ttl', 'http://solo.wst.univie.ac.at/test.ttl' ]
p s.namespaces # array of namespaces
p s.namespaces << RDF::SPARQL::Smart::Namespace.new('w', 'bla')


r = s.execute

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
