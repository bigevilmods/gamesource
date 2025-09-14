#!/corp/local/bin/tclsh8.0

lappend auto_path /corp/AMD/3Dtools/tcl_tk/libs

set list [lsort [glob EA3grammar*.txt]]
puts "list = $list"
set num [llength $list]
set fout [open EA3grammar.h w]
puts $fout {static const char *ea3grammarVersion = "$Id: $";}
puts $fout [subst {
class EA3_Grammar_Str \{
public:
  EA3_Grammar_Str();
  int num;
  char *data\[$num\];
  char *date\[$num\];
\};

EA3_Grammar_Str EA3_grammar_str;

EA3_Grammar_Str::EA3_Grammar_Str() \{
}]

set count 0
foreach fn $list {
  puts "converting $fn"
  puts $fout "  num = $num;"
  puts $fout "  static const char [file root $fn]\[\] = \{"
  set fin [open $fn r]
  set indata [read $fin]
  set date [clock format [file mtime $fn]]
  set data {}
  foreach line [split $indata \n] {
    if {[string index [string trim $line] 0]=="#"} continue
    append data $line\n
  }
  #puts $data
  set i 0
  puts -nonewline $fout "    ";
  foreach byte [split $data {}] {
    scan $byte %c byte
    puts -nonewline $fout "[format 0x%.2x $byte],"
    incr i
    if {$i == 10} {
      puts $fout ""
      puts -nonewline $fout "    ";
      set i 0
    }
  }
  puts $fout "0x00"
  puts $fout "  \};"
  puts $fout "  data\[$count\] = const_cast<char *>([file root $fn]);"
  puts $fout "  date\[$count\] = \"$date\";"
  incr count;
}
puts $fout "\}"
