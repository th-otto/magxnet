#! /usr/bin/perl
# a quick hack to turn `cvs log' output for a source tree into something
# more useful for distribution as ChangeLog file... (group changes by
# log message, sort by date; use author from log message if looks like
# mail/news article...)
# From: Juergen Lock "I like perl" <nox@jelal.hb.north.de>

# common root directory, remove from filenames
$src = "libc/";
# author to replace for `root', just in case...
$defauthor = "nox";

while ($_ = $ARGV[0], /^-/) {
	shift;
	last if /^--$/;
	/^-l/ && (++$list);
}
while (<>) {
	if ($lline = (/^-------*$/ .. /^=========*$/)) {
		if (($lline =~ /E0/ || /^-------*$/) && $text) {
			if ($lline != 1 || !($text =~ /^=========*$/)) {
				if (!($text =~ /^(From:?|Date:|Subject:|Message-I[Dd]:|References:)\s+/)) {
					$author = $defauthor if ($author == "root");
					$text = "X-From: $author\n\n" . $text;
				}
				$dates{$text} = $date if (!$dates{$text} ||
							$dates{$text} < $date);
				$file = $dir . $file;
				undef $dir;
				if (($files{$text} =~ /(.*\n)*(.*)/) &&
						(length $2) + (length $file) > 60) {
					$files{$text} .= "\n\t" . $file ;
				} else {
					$files{$text} .= ' ' . $file ;
				}
			}
			undef $text;
		}
		if ($hline = (/^-------*$/ .. /^date:.*$/)) {
			if (/^date:\s+(.*;\s+author:\s+(\S+);.*)$/) {
				$date = $1;
				$author = $2;
			}
		} else {
			$text .= $_ if ($text || !/^branches:\s+.*/);
		}
		if ($list) {
			if ($lline == 1) {
				print "$dir$file:\n\n"
			} else {
				print;
			}
		}
	} elsif (/^RCS file:\s+.*\/$src(\S+\/)[^\s\/]*$/) {
		$dir = $1;
	} elsif (/^Working file:\s+(\S+)$/) {
		$file = $1;
	}
}
foreach $text (keys %dates) {
	$xhead = "Index:$files{$text}\n";
	if ($text =~ /^X-From:/) {
		$_ = $dates{$text};
		s/;.*//;
		$xhead .= "X-Date: $_\n";
	}
	$out{$dates{$text}} = $xhead . $text;
}
undef $files, $dates;
foreach $key (sort(keys %out)) {
	print $out{$key};
	print "-------------------------------------------------------------\n";
}
