require 'mkmf'

PKG_NAME="rdf/smart"

$CFLAGS = '-g -Wall ' + $CFLAGS
$LIBPATH.push(RbConfig::CONFIG['libdir'])

unless have_library("raptor2")
  pp "Library raptor2 not found"
  exit 1
end
unless have_library("rasqal")
  pp "Library rasqal not found"
  exit 1
end
$CFLAGS = ' -std=c99 ' + `rasqal-config --cflags`.chomp + $CFLAGS
$LDFLAGS = ' ' + `rasqal-config --libs`.chomp + $LDFLAGS

create_header()
create_makefile(PKG_NAME)
