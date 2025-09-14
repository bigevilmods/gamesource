#!/corp/local/bin/tclsh8.0

lappend auto_path /corp/AMD/3Dtools/tcl_tk/libs

set cpp [open EA3names.cxx w]
set hpp [open ../include/EA3names.h w]

puts $cpp {#include "EA3names.h"}
puts $hpp {#ifndef _EA3NAMES_H_INCL}
puts $hpp {#define _EA3NAMES_H_INCL}
puts $hpp {#include "EA3.h"}
puts $hpp {//#include "EA3Export.h"}
puts $hpp {//#include "EA3Util.h"}

proc forfilelines {l fname code} {
# a for loop that loops thru each line is a file
# example:
# forfilelines line /usr/tmp/x {
#   puts $line
# }
  set f [open $fname]
  upvar $l line
  while {![eof $f]} {
    gets $f line
    if [eof $f] break
    uplevel 1 $code
  }
  close $f
}

forfilelines name names.txt {
  if {[string length $name]==0} continue
  if {[info exists Names($name)]} continue
  set Names($name) 1
  lappend NameList $name
}

puts $hpp "extern EA3_EXPORT XbIdentifier XB_names\[\];"
puts $cpp "EA3_EXPORT XbIdentifier XB_names\[\] = \{"
foreach name $NameList {
  puts $cpp "\"$name\"\,"
}
puts $cpp "\" \""
puts $cpp "};"
set i 0
foreach name $NameList {
  puts $hpp "#define XB$name XB_names\[$i\]"
  incr i
}
puts $hpp "#define XB XB_names\[$i\]"
puts $hpp "#endif // ifndef _EA3NAMES_H_INCL"

