cd `dirname $0`
f=hyp-tutorial
pdflatex $f
bibtex $f
pdflatex $f
pdflatex $f
mv hyp-tutorial*pdf ../../hyp-tutorial.pdf
