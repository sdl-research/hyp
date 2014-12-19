cd `dirname $0`
for f in hyp-tutorial.*; do
    if [[ $f != hyp-tutorial.tex ]] ; then
        rm $f
    fi
done
f=hyp-tutorial
pdflatex $f
bibtex $f
pdflatex $f
pdflatex $f
mv hyp-tutorial*pdf ../../hyp-tutorial.pdf
