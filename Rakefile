require 'rubygems'                                                                                       
require 'rake/clean'
require 'rubygems/package_task'
require 'rdoc/rdoc'

spec = eval(File.read('rsm.gemspec'))

task :default => [:config, :compile]

desc "Configuring library"
task :config do
  ruby 'ext/extconf.rb'
end

desc "Compiling library"
task :compile => :config do
  system "make"
end

Gem::PackageTask.new(spec) do |pkg|
  pkg.need_zip = true
  pkg.need_tar = true
end

desc "Installing library"
task :install => [:config,:compile,:gem] do
  system "sudo gem install pkg/rsm-#{spec.version.to_s}"
end
