
./build_and_run_ut.sh

cd build_ut

if [[ ! -d "html/" ]]
then
    echo "creating html dir"
    mkdir html
fi

gcovr -f ../sw/ -e='.*(T|t)est(s?)/' --branches -b --exclude-unreachable-branches --html-details html/test.html