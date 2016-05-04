require "bundler/gem_tasks"
require 'rake/extensiontask'
require 'rake/testtask'

Rake::ExtensionTask.new('scrub') do |ext|
  ext.name = 'scrub'
  ext.ext_dir = 'ext/string'
  ext.lib_dir = 'lib/string'
end
Rake::TestTask.new {|t| t.libs << 'test' }

task :default => [:compile, :test]
