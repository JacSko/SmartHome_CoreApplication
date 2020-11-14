
./build_and_run_ut.sh

cd build_ut

if [[ ! -d "html/" ]]
then
    echo "creating html dir"
    mkdir html
fi

gcovr -f ../sw/ --root=. -e /.*/test/.* -e /.*/tests/.* --html --html-details --output=html/coverage.html --exclude-unreachable-branches
firefox html/coverage.html
