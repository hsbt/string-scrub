# coding: utf-8
lib = File.expand_path('../lib', __FILE__)

Gem::Specification.new do |spec|
  spec.name          = "string-scrub"
  spec.version       = "0.0.1"
  spec.authors       = ["SHIBATA Hiroshi"]
  spec.email         = ["shibata.hiroshi@gmail.com"]
  spec.summary       = %q{String#scrub for Ruby 2.0.0}
  spec.description   = %q{String#scrub for Ruby 2.0.0}
  spec.homepage      = "https://github.com/hsbt/string-scrub"
  spec.license       = "MIT"

  spec.files         = `git ls-files`.split($/)
  spec.extensions    = ["ext/string/extconf.rb"]
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.required_ruby_version     = '>= 2.0.0'

  spec.add_development_dependency "bundler"
  spec.add_development_dependency "rake"
end
