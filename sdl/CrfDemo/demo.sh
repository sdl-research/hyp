HYPSDL=`dirname $0`
BUILD=${BUILD:-Release}
optimize=${1:-optimize}
built=$BUILD/Optimization/Optimize
if [[ -x $built ]] ; then
    optimize=$built
else
    optimize=`which $optimize`
fi
 --model CrfDemo \
  --config $HYPSDL/CrfDemo/example/config.yml \
  --search-space search-space-train \
  --optimize optimize-lbfgs
$BUILD/Optimization/Optimize --model $HYPSDL/CrfDemo \
  --config $HYPSDL/CrfDemo/example/config.yml \
  --search-space search-space-test \
  --test-mode
