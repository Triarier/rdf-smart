Gem::Specification.new do |s|
  s.name = 'rsm'
  s.version = '0.0.1'
  s.date = '2014-06-13'
  s.summary ="Hola!"
  s.description ="A simple gem. Hurra!"
  s.authors = ["Florian Stertz"]
  s.email = ['florian.stertz@gmail.com']
  s.files = Dir['ext/*']
  s.platform = Gem::Platform::RUBY
  s.require_paths = ['ext']
  s.extensions = Dir['ext/extconf.rb']
  s.homepage = 'https://github.com/Triarier/rdf-sparql-smart'
end
