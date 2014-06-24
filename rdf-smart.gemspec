Gem::Specification.new do |s|
  s.name = 'rdf-smart'
  s.version = '0.0.152'
  s.date = '2014-06-17'
  s.summary ="RDF.rb segfaults. A lot. We don't. RDF.rb has lots of features. We don't."
  s.description ="Minimal sparql support for ruby. Basically the roqet tool from http://librdf.org, with always guessing input type, and always json output."
  s.authors = ["Florian Stertz", "Juergen 'eTM' Mangler"]
  s.email = ['florian.stertz@gmail.com', 'juergen.mangler@gmail.com']
  s.files = Dir['ext/**/*'] + Dir['lib/**/*'] + ['rdf-smart.gemspec','README.md']
  s.platform = Gem::Platform::RUBY
  s.require_paths = ['lib']
  s.extensions = Dir['ext/extconf.rb']
  s.homepage = 'https://github.com/Triarier/rdf-smart'
end
