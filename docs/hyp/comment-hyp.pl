#!/usr/bin/perl -w 

while (<>) {
    s/(#.+)/\\begin{scriptsize}$1\\end{scriptsize}/;
    print;
}
