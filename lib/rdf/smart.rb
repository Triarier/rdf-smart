require File.dirname(__FILE__)+ '/../../ext/smart'
require 'json'
require 'pp'

##
# Main Module
module RDF
  ##
  # Creates a RDF::Smart Object. 
  class Smart
    ##
    # Exetuces a +query+. +query+ is a string.
    #
    # An Error is raised if a +data_source+ does not exist
    ##
    # A JSON Object will be returned.
    def execute(query) #{{{
      data_sources.each do |s|
        raise 'BlaBlaBla' unless File.exists?(s)
      end
      ## 
      # +res+ is the returned value.
      (res = JSON::parse(__execute(query)))['results']['bindings'].each do |solution|
        solution.each do |k,v|
          catch :done do
          ##
          # Convert Strings into the right datatype.
          ##
          # Add +pvalue+ into returned Value. +pvalue+ contains the real value without namespaces. 
            case v['datatype']
              when 'http://www.w3.org/2001/XMLSchema#integer', 'http://www.w3.org/2001/XMLSchema#long', 'http://www.w3.org/2001/XMLSchema#int', 'http://www.w3.org/2001/XMLSchema#short', 'http://www.w3.org/2001/XMLSchema#long', 'http://www.w3.org/2001/XMLSchema#nonPositiveInteger', 'http://www.w3.org/2001/XMLSchema#negativeInteger', 'http://www.w3.org/2001/XMLSchema#nonNegativeInteger', 'http://www.w3.org/2001/XMLSchema#positiveInteger', 'http://www.w3.org/2001/XMLSchema#unsignedLong', 'http://www.w3.org/2001/XMLSchema#unsignedInt', 'http://www.w3.org/2001/XMLSchema#unsignedShort'
                v['ns'] = v['datatype']
                v['pvalue'] = v['value'].to_i
                throw :done
              when 'http://www.w3.org/2001/XMLSchema#float', 'http://www.w3.org/2001/XMLSchema#decimal', 'http://www.w3.org/2001/XMLSchema#double'
                v['ns'] = v['datatype']
                v['pvalue'] = v['value'].to_f
                throw :done
              when 'http://www.w3.org/2001/XMLSchema#boolean'
                v['ns'] = v['datatype']
                v['pvalue'] = v['value'] == 'true' ? true : false
                throw :done
              when 'http://www.w3.org/2001/XMLSchema#notation', 'http://www.w3.org/2001/XMLSchema#qname'
                v['ns'] = v['datatype']
                v['pvalue'] = v['value']
                throw :done
              when nil
              else
                v['ns'] = v['datatype']
                v['pvalue'] = v['value']
                throw :done
            end
            if v.length == 2 && v['value'] && v['type'] == 'literal'
              v['datatype'] = 'http://www.w3.org/2001/XMLSchema#string'
              v['ns'] = v['datatype']
              v['pvalue'] = v['value']
              throw :done
            end
            if v['value'] == 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type'
              v['ns'] = v['value']
              v['pvalue'] = 'a'
              throw :done
            end
            namespaces.each do |kk, vv|
              if v['value'].start_with?(vv)
                v['ns'] = vv
                v['pvalue'] = v['value'].sub(vv,'')
                throw :done
              end
            end
          end  
        end
      end
      res #}}}
    end
  end
end
