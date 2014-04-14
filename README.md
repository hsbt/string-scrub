# String#scrub

[![Gem Version](https://badge.fury.io/rb/string-scrub.png)](http://badge.fury.io/rb/string-scrub)
[![Build Status](https://secure.travis-ci.org/hsbt/string-scrub.png)](http://travis-ci.org/hsbt/string-scrub)

String#scrub for Ruby 2.0.0 and 1.9.3

## What does the scrub method do?
If the given string contains an invalid byte sequence then that invalid byte sequence is replaced with the [unicode replacement character](http://www.fileformat.info/info/unicode/char/0fffd/index.htm) (�) and a new string is returned.

You may also set a replacement character of your choice by passing that character to the method.

## Usage

See [testcase](https://github.com/hsbt/string-scrub/blob/master/test/test_scrub.rb)

## Installation

Add this line to your application's Gemfile:

    gem 'string-scrub'

And then execute:

    $ bundle

Or install it yourself as:

    $ gem install string-scrub

## Contributing

1. Fork it
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -am 'Add some feature'`)
4. Push to the branch (`git push origin my-new-feature`)
5. Create new Pull Request
