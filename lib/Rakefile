require 'rubygems'                                                                                       
require 'rake/clean'
require 'rubygems/package_task'
require 'rdoc/rdoc'

task :default => [:config, :compile]

desc "Configuring library"
task :config do
  ruby 'extconf.rb'
end

desc "Compiling library"
task :compile => :config do
  system "make"
end
