#!/bin/bash

# Format comments so we can iclude them using \input in latex:
# for i in *.hyp; do n=$(basename $i .hyp)_include.hyp; ../comment-hyp.pl $i > $n; done 

# Produce hypergraph PDFs:
for i in *.hyp; do n=$(basename $i .hyp);  /local/xmt/xmt-git/Release/Hypergraph/HgDraw $i | dot -Tpdf > ${n}0.pdf; pdfcrop ${n}0.pdf $n.pdf; rm ${n}0.pdf; done 

# fstcompile --acceptor --isymbols=sent.syms sent.fst-txt | fstdraw --isymbols=sent.syms --acceptor | dot -Tpdf > sent.pdf 
