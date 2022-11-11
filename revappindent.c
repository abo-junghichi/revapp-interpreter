#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
typedef struct {
    size_t cur, max;
    char *buf;
} char_buf;
static void lineget(FILE * stream, char_buf * out)
{
    int c;
    out->cur = 0;
    while (1) {
	c = fgetc(stream);
	switch (c) {
	case '\n':
	case EOF:
	    return;
	default:{
		size_t nmemb = out->cur + 1, enough = nmemb * 2;
		if (out->max < nmemb) {
		    char *newbuf = realloc(out->buf, enough);
		    out->max = enough;
		    out->buf = newbuf;
		}
		out->buf[out->cur] = c;
		out->cur = nmemb;
	    }
	}
    }
}
typedef enum {
    token_ref, token_def, token_open, token_close
} token_t;
static int printline_stem(size_t head, size_t tail, char *buf)
{
    size_t i;
    for (i = head; i < tail; i++) {
	char peek = buf[i];
	if (isspace(peek) || '(' == peek || ')' == peek || '=' == peek)
	    break;
	fputc(peek, stdout);
    }
    return i;
}
static void printline(token_t before, size_t head, size_t tail, char *buf)
{
    size_t i;
    for (i = head; i < tail;) {
	char peek = buf[i];
	token_t after;
	if (isspace(peek)) {
	    i++;
	    continue;
	}
	switch (peek) {
	case '(':
	    after = token_open;
	    break;
	case ')':
	    after = token_close;
	    break;
	case '=':
	    after = token_def;
	    break;
	default:
	    after = token_ref;
	    break;
	}
	if ((token_open != before) && (token_close != after)
	    && ((token_def == before) || (token_def != after)))
	    fputc(' ', stdout);
	if (token_ref != after) {
	    fputc(peek, stdout);
	    i++;
	}
	if (token_ref == after || token_def == after)
	    i = printline_stem(i, tail, buf);
	before = after;
    }
    fputc('\n', stdout);
}
static void printindent(char c, size_t indent)
{
    size_t i;
    for (i = 0; i < indent; i++)
	fputc(c, stdout);
}
int main(void)
{
    size_t i;
    size_t indent = 0;
    char_buf line;
    line.cur = line.max = 0;
    line.buf = NULL;
    while (1) {
	size_t n;
	size_t head, tail;
	size_t head_cket = 0;
	token_t head_token = token_open;
	size_t cur_indent;
	lineget(stdin, &line);
	if (0 >= line.cur && feof(stdin))
	    break;
	for (i = 0; i < line.cur; i++)
	    if (!isspace(line.buf[i]))
		break;
	if (i >= line.cur) {
	    fputc('\n', stdout);
	    continue;
	}
	for (; i < line.cur; i++) {
	    char peek = line.buf[i];
	    if (')' == peek) {
		if (indent > head_cket) {
		    head_cket++;
		} else
		    break;
	    } else if (!isspace(peek))
		break;
	}
	indent -= head_cket;
	printindent(' ', indent);
	if (head_cket) {
	    printindent(')', head_cket);
	    head_token = token_close;
	}
	cur_indent = indent;
	head = i;
	tail = line.cur;
	n = 0;
	for (; i < line.cur; i++)
	    switch (line.buf[i]) {
	    case '(':
		if (0 >= n)
		    tail = i;
		n++;
		break;
	    case ')':
		if (n > 0)
		    n--;
		else if (indent > 0)
		    indent--;
		break;
	    }
	if (cur_indent > indent && n > 0) {
	    printline(head_token, head, tail, line.buf);
	    printindent(' ', indent);
	    head = tail;
	}
	printline(head_token, head, line.cur, line.buf);
	indent += n;
    }
    free(line.buf);
    return 0;
}
