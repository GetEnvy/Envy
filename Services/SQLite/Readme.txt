SQLite Amalgamation
http://sqlite.org/download.html

Notes for ease of updates comparison.


Regular Expressions (Regex)
From
To

(Note dot matches newline)



Cleanup:

[ \t]+$

(?<=[^\r])\n
\r\n



Comment Removal:

  In place of\r\n\*\* a legal notice, here is a blessing:\r\n\*\*\r\n\*\*    May you do good and not evil\.\r\n\*\*    May you find forgiveness for yourself and forgive others\.\r\n\*\*    May you share freely, never taking more than you give\.

\*\* CAPI3REF: ([^\r]+$).*?\*+/
** $1\r\n*/\r\n

\*\* Synopsis: [^\r]+\r\n



File Removal:

^/\*+ Begin file (FILENAME\.c) \*+\/\r\n.+^/\*+ End of FILENAME\.c \*+\/\r\n
/************** Removed file $1 *****************************************/

FILENAME:

mem2.c
mem3.c
mem5.c
fts3.c
fts5.c
fts5.h
fts3_aux.c
fts3_expr.c
fts3_hash.c
fts3_porter.c
fts3_tokenizer.c
vxworks.h
os_unix.c
mutex_unix.c

notify.c
fts3_unicode2.c
rtree.c
icu.c
fts3_icu.c
sqlite3rbu.c
sqlite3session.c
sqlite3session.h
dbstat.c
json1.c
