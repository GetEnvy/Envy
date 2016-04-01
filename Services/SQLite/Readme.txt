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

(^/\*+ Begin file mutex_unix.c \*+\/\r\n).+(^/\*+ End of mutex_unix.c \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_MUTEX_PTHREADS Content Removed */\r\n\r\n$2

(^/\*+ Begin file os_unix.c \*+\/\r\n).+(^/\*+ End of os_unix.c \*+\/\r\n)
$1\r\n/* #if SQLITE_OS_UNIX Content Removed */\r\n\r\n$2

(^/\*+ Begin file notify.c \*+\/\r\n).+(^/\*+ End of notify.c \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_ENABLE_UNLOCK_NOTIFY Content Removed */\r\n\r\n$2

(^/\*+ Begin file fts3.c \*+\/\r\n).+(^/\*+ End of fts3_[^ ]+ \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_ENABLE_FTS3 Content Removed */\r\n\r\n$2

(^/\*+ Begin file mem3.c \*+\/\r\n).+(^/\*+ End of mem3[^ ]+ \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_ENABLE_MEMSYS3 Content Removed */\r\n\r\n$2

(^/\*+ Begin file mem5.c \*+\/\r\n).+(^/\*+ End of mem5[^ ]+ \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_ENABLE_MEMSYS5 Content Removed */\r\n\r\n$2

(^/\*+ Begin file sqlite3rbu.c \*+\/\r\n).+(^/\*+ End of sqlite3rbu[^ ]+ \*+\/\r\n)
$1\r\n/* RBU Extension Content Removed */\r\n\r\n$2

(^/\*+ Begin file dbstat.c \*+\/\r\n).+(^/\*+ End of dbstat[^ ]+ \*+\/\r\n)
$1\r\n/* dbstat Virtual Table Content Removed */\r\n\r\n$2

(^/\*+ Begin file json1.c \*+\/\r\n).+(^/\*+ End of json1.c \*+\/\r\n)
$1\r\n/* JSON Content Removed */\r\n\r\n$2

(^/\*+ Begin file fts5.c \*+\/\r\n).+(^/\*+ End of fts5.c \*+\/\r\n)
$1\r\n/* #ifdef SQLITE_ENABLE_FTS5 Content Removed */\r\n\r\n$2
