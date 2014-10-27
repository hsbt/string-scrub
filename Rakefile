require "bundler/gem_tasks"
require 'rake/extensiontask'
require 'rake/testtask'

Rake::ExtensionTask.new('string-scrub')
Rake::TestTask.new {|t| t.libs << 'test' }
