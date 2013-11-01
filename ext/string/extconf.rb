require 'mkmf'
have_func("rb_str_scrub")
create_makefile('string/scrub')
