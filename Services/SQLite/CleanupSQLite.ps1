# CleanupSQLite.ps1
# Modified Jan 2018
# From Raw Amalgamation: Strip Trailing Whitespace/Comments/EOL/etc.

# Allow Drag&Drop: (Files onto script)
#HKEY_CLASSES_ROOT\Microsoft.PowerShellScript.1\ShellEx\DropHandler\
#(Default) = {60254CA5-953B-11CF-8C96-00AA00B8708C}

# Run on double-click:
#HKEY_CLASSES_ROOT\Microsoft.PowerShellScript.1\Shell\Open\Command\
#(Default) = "C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe" -NoExit -File "%1" %*

#set-executionpolicy unrestricted

#requires -version 3

begin {
	# Header:
	$copyright = @"
/*
** sqlite3.c  (3.2x.0) (20xx-xx-xx)
**
** This file is part of Envy (getenvy.com) © 2016-2018
** The original author disclaimed copyright to this source code.
*/

"@
	$options = @"
/* Envy-Specific Options, See http://www.sqlite.org/compile.html */


#define SQLITE_OMIT_DEPRECATED

#define SQLITE_OMIT_WAL	1				/* Write-Ahead Log (3.7.0+), 20kb + unwanted tempfiles */
#define SQLITE_OMIT_VIRTUALTABLE		/* Virtual Tables, 14kb (Requires custom fix) */
#define SQLITE_OMIT_ALTERTABLE			/* Modify existing tables (3.1.4+), 5kb (Custom fix) */
#define SQLITE_OMIT_ANALYZE				/* ANALYZE command, 4kb (Custom fix) */
#define SQLITE_OMIT_AUTHORIZATION		/* Authorize, 4kb */
#define SQLITE_OMIT_AUTOMATIC_INDEX		/* Temp index for single statement, 2kb */
#define SQLITE_OMIT_AUTOINCREMENT		/* Alt ROWID, 2kb */
#define SQLITE_OMIT_AUTOVACUUM			/* Pragma auto_vacuum, 9kb */
#define SQLITE_OMIT_CHECK				/* CHECK constraints, 1kb */
#define SQLITE_OMIT_COMPILEOPTION_DIAGS /* Compile-time diagnostics, 2kb */
#define SQLITE_OMIT_COMPOUND_SELECT		/* SELECT: UNION, INTERSECT, EXCEPT, 6kb */
#define SQLITE_OMIT_COMPLETE			/* sqlite3_complete(), 1kb */
#define SQLITE_OMIT_DATETIME_FUNCS		/* date() time() datetime() julianday(), 5kb */
#define SQLITE_OMIT_DECLTYPE			/* sqlite3_column_decltype(), 1kb */
#define SQLITE_OMIT_EXPLAIN 			/* Keyword "EXPLAIN", 5kb */
#define SQLITE_OMIT_FOREIGN_KEY			/* Foreign Key Constraint, 10kb */
#define SQLITE_OMIT_GET_TABLE			/* sqlite3_get_table(), 1kb */
#define SQLITE_OMIT_INCRBLOB			/* sqlite3_blob_open(), 4kb */
#define SQLITE_OMIT_INTEGRITY_CHECK		/* Pragma integrity_check, 8kb */
#define SQLITE_OMIT_LOAD_EXTENSION		/* sqlite3_load_extension(), 2kb */
#define SQLITE_OMIT_LOOKASIDE			/* ~1kb */
#define SQLITE_OMIT_LIKE_OPTIMIZATION	/* WHERE: LIKE, GLOB, 2kb */
#define SQLITE_OMIT_OR_OPTIMIZATION		/* WHERE: OR, 1kb */
#define SQLITE_OMIT_PROGRESS_CALLBACK	/* sqlite3_progress_handler(), 1kb */
#define SQLITE_OMIT_REINDEX				/* REINDEX, 1kb (Custom fix) */
#define SQLITE_OMIT_SUBQUERY			/* IN() operator, 6kb (Custom fix) */
#define SQLITE_OMIT_MEMORYDB			/* ":memory:" database, 1kb */
#define SQLITE_OMIT_TEMPDB				/* Temporary Tables, 1kb */
#define SQLITE_OMIT_TRACE				/* sqlite3_profile() sqlite3_trace(), 2kb */
#define SQLITE_OMIT_TRIGGER				/* TRIGGER objects, 11kb (Custom fix) */
#define SQLITE_OMIT_XFER_OPT			/* "INSERT INTO ... SELECT ...", 2kb */
#define SQLITE_OMIT_BUILTIN_TEST		/* Obsolete, Renamed SQLITE_UNTESTABLE below */
#define SQLITE_UNTESTABLE				/* Was SQLITE_OMIT_BUILTIN_TEST, sqlite3_test_control(), 2kb */

#define SQLITE_OMIT_FLAG_PRAGMAS		/* Pragma, 6kb, Not needed for synchronous=off */
#define SQLITE_OMIT_SCHEMA_PRAGMAS
#define SQLITE_OMIT_SCHEMA_VERSION_PRAGMAS
/* #define SQLITE_OMIT_PAGER_PRAGMAS */	/* Needed for journal_mode=off */

/* #define SQLITE_OMIT_BTREECOUNT */	/* 0kb */
/* #define SQLITE_OMIT_CAST */			/* 0kb */
/* #define SQLITE_OMIT_CTE */			/* WITH (Common table expressions), 5kb (fails) */
/* #define SQLITE_OMIT_LOCALTIME */		/* "localtime", 0kb */
/* #define SQLITE_OMIT_TCL_VARIABLE */	/* "$" binding syntax, 0kb */
/* #define SQLITE_OMIT_FLOATING_POINT */ /* Literal float will parse error, 5kb */	/* Build warnings */
/* #define SQLITE_OMIT_SHARED_CACHE */	/* sqlite3_enable_shared_cache(), 6kb (Custom fix required) */	/* Used */
/* #define SQLITE_OMIT_MERGE_SORT */	/* (3.7.8+), 5kb, (Removed 3.7.16+) */
/* #define SQLITE_OMIT_QUICKBALANCE */	/* 0kb */

/* #define SQLITE_WIN32_MALLOC */		/* Adds 1kb */
/* #define SQLITE_THREADSAFE 1 */		/* 0 mutexes locked, 1 serialized for threadsafety, 2 multithreaded */
/* #define SQLITE_NO_SYNC */

#define SQLITE_DISABLE_PAGECACHE_OVERFLOW_STATS

/* End Envy-specifc options */

/*
** The author disclaims copyright to this source code.
** In place of a legal notice, here is a blessing:
**
**	May you do good and not evil.
**	May you find forgiveness for yourself and forgive others.
**	May you share freely, never taking more than you give.
*/
"@

	# End of Main:
	$mainfind = @"
(?ms)End of main.c \*.+\z
"@
	$mainreplace = @"
End of main.c ****/
"@

	# End of Header:
	$headerfind = @"
(?ms)Begin file sqlite3rtree.h \*.+\z
"@
	$headerreplace = @"
End of sqlite.h ****/
"@

	# Trailing Whitespace:
	$trailfind = '[ \t]+(\r\n)'
	$trailreplace = '$1'

	# Notice:
	$noticefind = @"
(?ms)  In place of\r\n\*\* a legal notice, here is a blessing:\r\n\*\*\r\n\*\*    May you do good and not evil\.\r\n\*\*    May you find forgiveness for yourself and forgive others\.\r\n\*\*    May you share freely, never taking more than you give\.
"@
	$noticereplace = ''

	# CAPI3REF:
	$capi3reffind = '(\*\* CAPI3REF: .*\r\n)(\*\* ?.*\r\n)+'
	$capi3refreplace = '$1'

	# Synopsis:
	$synopsisfind = '(\* Opcode:) .*(\r\n)(\*\* Synopsis: .*\r\n)?'
	$synopsisreplace = '$1$2'

	# Orphans:
	$orphanfind = @"
(?ms)\r\n *\*\* *([^.].{1,12}\.\r\n)
"@
	$orphanreplace = ' $1'

	# Remove Files:
	$filefind = @"
(?ms)/\*+ Begin file (os_unix.c|mutex_unix.c) (\*+\/\r\n)[^•]+/\*+ End of \1 \*+\/\r\n
"@
	$filereplace = '/************** Removed file $1 $2'

	$regextrail = [regex] $trailfind
	$regexcapi3ref = [regex] $capi3reffind
	$regexorphan = [regex] $orphanfind

	$opts = [System.Text.RegularExpressions.RegexOptions]::MultiLine
	#$regextrail = new-object Text.RegularExpressions.Regex($trailfind, $opts)
	$regexmain = new-object Text.RegularExpressions.Regex($mainfind, $opts)
	$regexheader = new-object Text.RegularExpressions.Regex($headerfind, $opts)
	$regexnotice = new-object Text.RegularExpressions.Regex($noticefind, $opts)
	$regexsynopsis = new-object Text.RegularExpressions.Regex($synopsisfind, $opts)
	$regexfile = new-object Text.RegularExpressions.Regex($filefind, $opts)
}

process {
	try {
		#$text = $textraw = [IO.File]::ReadAllText("sqlite3.h")
		$text = Get-Content "sqlite3.h" | Out-String

		$text = $regextrail.Replace($text, $trailreplace)
		$text = $regexheader.Replace($text, $headerreplace)
		$text = $regexnotice.Replace($text, $noticereplace)
		$text = $regexcapi3ref.Replace($text, $capi3refreplace)
		$text = $regexsynopsis.Replace($text, $synopsisreplace)
		$text = $regexorphan.Replace($text, $orphanreplace)

		if ($text -ne $textraw) {
			$text = $copyright + $text
			[IO.File]::WriteAllText("sqlite3.h.temp", $text, [Text.Encoding]::UTF8)
			copy-item sqlite3.h.temp sqlite3.h -force -erroraction Continue
			remove-item sqlite3.h.temp
			Write-Host "sqlite3.h Updated."
		}
	} catch [Management.Automation.MethodInvocationException] {
		write-error $ERROR[0]
	}

	try {
		#$text = $textraw = [IO.File]::ReadAllText("sqlite3.c")
		$text = Get-Content "sqlite3.c" | Out-String

		$text = $regextrail.Replace($text, $trailreplace)
		$text = $regexmain.Replace($text, $mainreplace)
		$text = $regexnotice.Replace($text, $noticereplace)
		$text = $regexcapi3ref.Replace($text, $capi3refreplace)
		$text = $regexsynopsis.Replace($text, $synopsisreplace)
		$text = $regexorphan.Replace($text, $orphanreplace)
		$text = $regexfile.Replace($text, $filereplace)

		if (!$text.equals($textraw)) {
			$text = $copyright + $options + $text
			[IO.File]::WriteAllText("sqlite3.c.temp", $text, [Text.Encoding]::UTF8)
			copy-item sqlite3.c.temp sqlite3.c -force -erroraction Continue
			remove-item sqlite3.c.temp
			Write-Host "sqlite3.c Updated."
		}
	} catch [Management.Automation.MethodInvocationException] {
		write-error $ERROR[0]
	}

	Write-Host "Done."
	pause
} # process

end { }