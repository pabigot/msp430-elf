#PROG: strip
#source: unique.s
#strip: -g
#readelf: -s
#name: strip on STB_GNU_UNIQUE
#skip: msp430-*-* hppa*-*-hpux*

#...
 +[0-9]+: +[0-9a-f]+ +[0-9]+ +OBJECT +(UNIQUE|<OS specific>: 10) +DEFAULT +[1-9] foo
