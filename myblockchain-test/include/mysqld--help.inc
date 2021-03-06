#
# myblockchaind --help
#
--source include/not_embedded.inc

#
# force lower-case-table-names=1 (linux/macosx have different defaults)
# force symbolic-links=0 (valgrind build has a different default)
#

exec $MYBLOCKCHAIND_BOOTSTRAP_CMD --symbolic-links=0 --lower-case-table-names=1 --help --verbose > $MYBLOCKCHAIN_TMP_DIR/myblockchaind--help.txt 2>/dev/null;

# The inline perl code below will copy $MYBLOCKCHAIN_TMP_DIR/myblockchaind--help.txt
# to output, but filter away some variable stuff (e.g. paths).

perl;
  # Variables which we don't want to display in the result file since
  # their paths may vary:
  @skipvars=qw/basedir open-files-limit general-log-file plugin-dir
               pid-file slow-query-log-file
               datadir slave-load-tmpdir tmpdir socket table-definition-cache table-open-cache
               secure-file-priv/;
               
  # Variables which we don't want to display in result file since
  # they depend on type of build
  @build_specific=qw/auto-generate-certs/;

  # Plugins which may or may not be there:
  @plugins=qw/innodb ngram mecab ndb ndbinfo archive blackhole federated partition ndbcluster debug temp-pool ssl des-key-file
              thread-concurrency super-large-pages mutex-deadlock-detector null-audit
              sha256-password-private-key-path sha256-password-public-key-path
              sha256-password-auto-generate-rsa-keys/;

  # And substitute the content some environment variables with their
  # names:
  @env=qw/MYBLOCKCHAINTEST_VARDIR MYBLOCKCHAIN_TEST_DIR MYBLOCKCHAIN_CHARSETSDIR MYBLOCKCHAIN_SHAREDIR/;

  $re1=join('|', @skipvars, @build_specific, @plugins);
  $re2=join('|', @build_specific, @plugins);
  $skip=0;
  open(F, '<', "$ENV{MYBLOCKCHAIN_TMP_DIR}/myblockchaind--help.txt") or die;
  while (<F>) {
    next if 1../The following groups are read/;
    # formatting, skip line consisting entirely of dashes and blanks
    next if /^[\- ]+\s?$/;
    next if /Value \(after reading options\)/; # skip table header
    next if /^($re1) /;
    next if /^($re2)-/;
    $skip=0 if /^  -/;
    $skip=1 if / --($re2)\b/;
    y!\\!/!;
    s/[ ]+/ /; # squeeze spaces to remove table formatting
    # fixes for 32-bit
    s/\b4294967295\b/18446744073709551615/;
    s/\b2146435072\b/9223372036853727232/;
    s/\b196608\b/262144/;
    # Replacing port number with string
    s/^port \d+/port ####/;
    foreach $var (@env) { s/$ENV{$var}/$var/ }
    next if /use --skip-(use-)?symbolic-links to disable/; # for valgrind, again
    next if $skip;
    print;
  }
  close F;
EOF

