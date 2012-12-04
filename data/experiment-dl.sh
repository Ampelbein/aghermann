set -e
URL="http://johnhommer.com/academic/code/aghermann/Experiment.tar.bz2"
TARBALL=Experiment.tar.bz2
echo "
Now downloading sample data.
Press Ctl-C to interrupt.
"
function kill_wget
{
    kill $!
}
trap kill_wget SIGTERM SIGINT
wget -c $URL &
wait %1

tar xjf $TARBALL
rm -f $TARBALL

echo "Sample data set downloaded and unpacked"
read -p "Press <Enter> to close this window..."
