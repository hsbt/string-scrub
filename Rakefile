require "bundler/gem_tasks"
require 'rake/extensiontask'
require 'rake/testtask'

Rake::ExtensionTask.new('string-scrub') do |ext|
  ext.name = 'scrub'
  ext.lib_dir = 'lib/string-scrub'
end
Rake::TestTask.new {|t| t.libs << 'test' }
