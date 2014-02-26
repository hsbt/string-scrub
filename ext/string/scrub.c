#include <ruby.h>
#include <ruby/encoding.h>

#ifndef HAVE_RB_STR_SCRUB

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#define STR_ENC_GET(str) rb_enc_from_index(ENCODING_GET(str))
#ifndef UNREACHABLE
# define UNREACHABLE
#endif

static inline const char *
search_nonascii(const char *p, const char *e)
{
#if SIZEOF_VALUE == 8
# define NONASCII_MASK 0x8080808080808080ULL
#elif SIZEOF_VALUE == 4
# define NONASCII_MASK 0x80808080UL
#endif
#ifdef NONASCII_MASK
    if ((int)sizeof(VALUE) * 2 < e - p) {
        const VALUE *s, *t;
        const VALUE lowbits = sizeof(VALUE) - 1;
        s = (const VALUE*)(~lowbits & ((VALUE)p + lowbits));
        while (p < (const char *)s) {
            if (!ISASCII(*p))
                return p;
            p++;
        }
        t = (const VALUE*)(~lowbits & (VALUE)e);
        while (s < t) {
            if (*s & NONASCII_MASK) {
                t = s;
                break;
            }
            s++;
        }
        p = (const char *)t;
    }
#endif
    while (p < e) {
        if (!ISASCII(*p))
            return p;
        p++;
    }
    return NULL;
}

static VALUE
str_compat_and_valid(VALUE str, rb_encoding *enc)
{
    int cr;
    str = StringValue(str);
    cr = rb_enc_str_coderange(str);
    if (cr == ENC_CODERANGE_BROKEN) {
#ifdef PRIsVALUE
	rb_raise(rb_eArgError, "replacement must be valid byte sequence '%+"PRIsVALUE"'", str);
#else
	str = rb_inspect(str);
	rb_raise(rb_eArgError, "replacement must be valid byte sequence '%s'", RSTRING_PTR(str));
	RB_GC_GUARD(str);
#endif
    }
    else if (cr == ENC_CODERANGE_7BIT) {
	rb_encoding *e = STR_ENC_GET(str);
	if (!rb_enc_asciicompat(enc)) {
	    rb_raise(rb_eEncCompatError, "incompatible character encodings: %s and %s",
		    rb_enc_name(enc), rb_enc_name(e));
	}
    }
    else { /* ENC_CODERANGE_VALID */
	rb_encoding *e = STR_ENC_GET(str);
	if (enc != e) {
	    rb_raise(rb_eEncCompatError, "incompatible character encodings: %s and %s",
		    rb_enc_name(enc), rb_enc_name(e));
	}
    }
    return str;
}

/**
 * @param str the string to be scrubbed
 * @param repl the replacement character
 * @return If given string is invalid, returns a new string. Otherwise, returns Qnil.
 */
static VALUE
str_scrub0(int argc, VALUE *argv, VALUE str)
{
    int cr = ENC_CODERANGE(str);
    rb_encoding *enc;
    int encidx;
    VALUE repl;

    if (cr == ENC_CODERANGE_7BIT || cr == ENC_CODERANGE_VALID)
	return Qnil;

    enc = STR_ENC_GET(str);
    rb_scan_args(argc, argv, "01", &repl);
    if (argc != 0) {
	repl = str_compat_and_valid(repl, enc);
    }

    if (rb_enc_dummy_p(enc)) {
	return Qnil;
    }
    encidx = rb_enc_to_index(enc);

#define DEFAULT_REPLACE_CHAR(str) do { \
	static const char replace[sizeof(str)-1] = str; \
	rep = replace; replen = (int)sizeof(replace); \
    } while (0)

    if (rb_enc_asciicompat(enc)) {
	const char *p = RSTRING_PTR(str);
	const char *e = RSTRING_END(str);
	const char *p1 = p;
	const char *rep;
	long replen;
	int rep7bit_p;
	VALUE buf = Qnil;
	if (rb_block_given_p()) {
	    rep = NULL;
	    replen = 0;
	    rep7bit_p = FALSE;
	}
	else if (!NIL_P(repl)) {
	    rep = RSTRING_PTR(repl);
	    replen = RSTRING_LEN(repl);
	    rep7bit_p = (ENC_CODERANGE(repl) == ENC_CODERANGE_7BIT);
	}
	else if (encidx == rb_utf8_encindex()) {
	    DEFAULT_REPLACE_CHAR("\xEF\xBF\xBD");
	    rep7bit_p = FALSE;
	}
	else {
	    DEFAULT_REPLACE_CHAR("?");
	    rep7bit_p = TRUE;
	}
	cr = ENC_CODERANGE_7BIT;

	p = search_nonascii(p, e);
	if (!p) {
	    p = e;
	}
	while (p < e) {
	    int ret = rb_enc_precise_mbclen(p, e, enc);
	    if (MBCLEN_NEEDMORE_P(ret)) {
		break;
	    }
	    else if (MBCLEN_CHARFOUND_P(ret)) {
		cr = ENC_CODERANGE_VALID;
		p += MBCLEN_CHARFOUND_LEN(ret);
	    }
	    else if (MBCLEN_INVALID_P(ret)) {
		/*
		 * p1~p: valid ascii/multibyte chars
		 * p ~e: invalid bytes + unknown bytes
		 */
		long clen = rb_enc_mbmaxlen(enc);
		if (NIL_P(buf)) buf = rb_str_buf_new(RSTRING_LEN(str));
		if (p > p1) {
		    rb_str_buf_cat(buf, p1, p - p1);
		}

		if (e - p < clen) clen = e - p;
		if (clen <= 2) {
		    clen = 1;
		}
		else {
		    const char *q = p;
		    clen--;
		    for (; clen > 1; clen--) {
			ret = rb_enc_precise_mbclen(q, q + clen, enc);
			if (MBCLEN_NEEDMORE_P(ret)) break;
			if (MBCLEN_INVALID_P(ret)) continue;
			UNREACHABLE;
		    }
		}
		if (rep) {
		    rb_str_buf_cat(buf, rep, replen);
		    if (!rep7bit_p) cr = ENC_CODERANGE_VALID;
		}
		else {
		    repl = rb_yield(rb_enc_str_new(p, clen, enc));
		    repl = str_compat_and_valid(repl, enc);
		    rb_str_buf_cat(buf, RSTRING_PTR(repl), RSTRING_LEN(repl));
		    if (ENC_CODERANGE(repl) == ENC_CODERANGE_VALID)
			cr = ENC_CODERANGE_VALID;
		}
		p += clen;
		p1 = p;
		p = search_nonascii(p, e);
		if (!p) {
		    p = e;
		    break;
		}
	    }
	    else {
		UNREACHABLE;
	    }
	}
	if (NIL_P(buf)) {
	    if (p == e) {
		ENC_CODERANGE_SET(str, cr);
		return Qnil;
	    }
	    buf = rb_str_buf_new(RSTRING_LEN(str));
	}
	if (p1 < p) {
	    rb_str_buf_cat(buf, p1, p - p1);
	}
	if (p < e) {
	    if (rep) {
		rb_str_buf_cat(buf, rep, replen);
		if (!rep7bit_p) cr = ENC_CODERANGE_VALID;
	    }
	    else {
		repl = rb_yield(rb_enc_str_new(p, e-p, enc));
		repl = str_compat_and_valid(repl, enc);
		rb_str_buf_cat(buf, RSTRING_PTR(repl), RSTRING_LEN(repl));
		if (ENC_CODERANGE(repl) == ENC_CODERANGE_VALID)
		    cr = ENC_CODERANGE_VALID;
	    }
	}
	ENCODING_CODERANGE_SET(buf, rb_enc_to_index(enc), cr);
	return buf;
    }
    else {
	/* ASCII incompatible */
	const char *p = RSTRING_PTR(str);
	const char *e = RSTRING_END(str);
	const char *p1 = p;
	VALUE buf = Qnil;
	const char *rep;
	long replen;
	long mbminlen = rb_enc_mbminlen(enc);
	if (!NIL_P(repl)) {
	    rep = RSTRING_PTR(repl);
	    replen = RSTRING_LEN(repl);
	}
	else if (!strcasecmp(rb_enc_name(enc), "UTF-16BE")) {
	    DEFAULT_REPLACE_CHAR("\xFF\xFD");
	}
	else if (!strcasecmp(rb_enc_name(enc), "UTF-16LE")) {
	    DEFAULT_REPLACE_CHAR("\xFD\xFF");
	}
	else if (!strcasecmp(rb_enc_name(enc), "UTF-32BE")) {
	    DEFAULT_REPLACE_CHAR("\x00\x00\xFF\xFD");
	}
	else if (!strcasecmp(rb_enc_name(enc), "UTF-32lE")) {
	    DEFAULT_REPLACE_CHAR("\xFD\xFF\x00\x00");
	}
	else {
	    DEFAULT_REPLACE_CHAR("?");
	}

	while (p < e) {
	    int ret = rb_enc_precise_mbclen(p, e, enc);
	    if (MBCLEN_NEEDMORE_P(ret)) {
		break;
	    }
	    else if (MBCLEN_CHARFOUND_P(ret)) {
		p += MBCLEN_CHARFOUND_LEN(ret);
	    }
	    else if (MBCLEN_INVALID_P(ret)) {
		const char *q = p;
		long clen = rb_enc_mbmaxlen(enc);
		if (NIL_P(buf)) buf = rb_str_buf_new(RSTRING_LEN(str));
		if (p > p1) rb_str_buf_cat(buf, p1, p - p1);

		if (e - p < clen) clen = e - p;
		if (clen <= mbminlen * 2) {
		    clen = mbminlen;
		}
		else {
		    clen -= mbminlen;
		    for (; clen > mbminlen; clen-=mbminlen) {
			ret = rb_enc_precise_mbclen(q, q + clen, enc);
			if (MBCLEN_NEEDMORE_P(ret)) break;
			if (MBCLEN_INVALID_P(ret)) continue;
			UNREACHABLE;
		    }
		}
		if (rep) {
		    rb_str_buf_cat(buf, rep, replen);
		}
		else {
		    repl = rb_yield(rb_enc_str_new(p, e-p, enc));
		    repl = str_compat_and_valid(repl, enc);
		    rb_str_buf_cat(buf, RSTRING_PTR(repl), RSTRING_LEN(repl));
		}
		p += clen;
		p1 = p;
	    }
	    else {
		UNREACHABLE;
	    }
	}
	if (NIL_P(buf)) {
	    if (p == e) {
		ENC_CODERANGE_SET(str, ENC_CODERANGE_VALID);
		return Qnil;
	    }
	    buf = rb_str_buf_new(RSTRING_LEN(str));
	}
	if (p1 < p) {
	    rb_str_buf_cat(buf, p1, p - p1);
	}
	if (p < e) {
	    if (rep) {
		rb_str_buf_cat(buf, rep, replen);
	    }
	    else {
		repl = rb_yield(rb_enc_str_new(p, e-p, enc));
		repl = str_compat_and_valid(repl, enc);
		rb_str_buf_cat(buf, RSTRING_PTR(repl), RSTRING_LEN(repl));
	    }
	}
	ENCODING_CODERANGE_SET(buf, rb_enc_to_index(enc), ENC_CODERANGE_VALID);
	return buf;
    }
}

/*
 *  call-seq:
 *    str.scrub -> new_str
 *    str.scrub(repl) -> new_str
 *    str.scrub{|bytes|} -> new_str
 *
 *  If the string is invalid byte sequence then replace invalid bytes with given replacement
 *  character, else returns self.
 *  If block is given, replace invalid bytes with returned value of the block.
 *
 *     "abc\u3042\x81".scrub #=> "abc\u3042\uFFFD"
 *     "abc\u3042\x81".scrub("*") #=> "abc\u3042*"
 *     "abc\u3042\xE3\x80".scrub{|bytes| '<'+bytes.unpack('H*')[0]+'>' } #=> "abc\u3042<e380>"
 */
VALUE
rb_str_scrub(int argc, VALUE *argv, VALUE str)
{
    VALUE new = str_scrub0(argc, argv, str);
    return NIL_P(new) ? rb_str_dup(str): new;
}

/*
 *  call-seq:
 *    str.scrub! -> str
 *    str.scrub!(repl) -> str
 *    str.scrub!{|bytes|} -> str
 *
 *  If the string is invalid byte sequence then replace invalid bytes with given replacement
 *  character, else returns self.
 *  If block is given, replace invalid bytes with returned value of the block.
 *
 *     "abc\u3042\x81".scrub! #=> "abc\u3042\uFFFD"
 *     "abc\u3042\x81".scrub!("*") #=> "abc\u3042*"
 *     "abc\u3042\xE3\x80".scrub!{|bytes| '<'+bytes.unpack('H*')[0]+'>' } #=> "abc\u3042<e380>"
 */
static VALUE
str_scrub_bang(int argc, VALUE *argv, VALUE str)
{
    VALUE new = str_scrub0(argc, argv, str);
    if (!NIL_P(new)) rb_str_replace(str, new);
    return str;
}

#endif

void
Init_scrub(void)
{
#ifndef HAVE_RB_STR_SCRUB
    rb_define_method(rb_cString, "scrub", rb_str_scrub, -1);
    rb_define_method(rb_cString, "scrub!", str_scrub_bang, -1);
#endif
}
