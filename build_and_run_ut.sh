
if [[ ! -d "build_ut/" ]]
then
    echo "creating build_ut dir"
    mkdir build_ut
fi

cd build_ut

cmake .. -DUNIT_TESTS=On

make

ctest --output-on-failure --schedule-random --timeout 60