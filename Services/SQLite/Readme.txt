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

([^*])\r\n *\*\* +([a-zA-Z0-9) ]{1,11}\.\)?)$
$1 $2



File Removal:

End of main.c \*.+\z
End of main.c ***/


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
fts3_unicode2.c
vxworks.h
os_unix.c
mutex_unix.c
rtree.c
sqlite3session.c
sqlite3session.h
