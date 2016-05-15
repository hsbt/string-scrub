# coding: utf-8

Gem::Specification.new do |spec|
  spec.name          = "string-scrub"
  spec.version       = "0.1.2"
  spec.authors       = ["SHIBATA Hiroshi"]
  spec.email         = ["hsbt@ruby-lang.org"]
  spec.summary       = %q{String#scrub for Ruby 1.9.3 - 2.2.x}
  spec.description   = %q{String#scrub for Ruby 1.9.3 - 2.2.x}
  spec.homepage      = "https://github.com/hsbt/string-scrub"
  spec.license       = "2-clause BSDL"

  spec.files         = `git ls-files`.split($/)
  spec.extensions    = ["ext/string/extconf.rb"]
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.required_ruby_version     = ['>= 1.9.3', '< 2.3']

  spec.add_development_dependency "bundler"
  spec.add_development_dependency "rake"
  spec.add_development_dependency 'rake-compiler'
  spec.add_development_dependency "test-unit"
end
