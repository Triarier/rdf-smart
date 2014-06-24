require 'rubygems'                                                                                       
require 'rake/clean'
require 'rubygems/package_task'
require 'rdoc/rdoc'

spec = eval(File.read('rdf-smart.gemspec'))

task :default => [:config, :compile]

desc "Configuring library"
task :config do
  Dir.chdir('ext')  
  ruby 'extconf.rb'
  Dir.chdir(File.dirname(__FILE__))  
end

desc "Compiling library"
task :compile => :config do
  Dir.chdir('ext')  
  system "make"
  Dir.chdir(File.dirname(__FILE__))  
end

Gem::PackageTask.new(spec) do |pkg|
  pkg.need_zip = true
  pkg.need_tar = true
end

desc "Installing library"
task :install => [:config,:compile,:gem] do
  system "sudo gem install pkg/rdf-smart-#{spec.version.to_s}.gem"
end

desc "Pushing library"
task :push => [:config,:compile,:gem] do
  system "gem push pkg/rdf-smart-#{spec.version.to_s}.gem"
end


