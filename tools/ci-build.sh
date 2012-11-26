export CCACHE_DIR=/var/tmp/ccache/
cmake -DCMAKE_CXX_COMPILER=/usr/lib/ccache/bin/clang++ -DCMAKE_C_COMPILER=/usr/lib/ccache/bin/clang .
make

echo "Run the test suite"
./bin/boosttest--eddic_boost_test --report_level=detailed --report_format=xml --report_sink=test-results.xml

echo "Analyze the source code with cppcheck"
cppcheck -v --platform=unix64 --std=c++11 --enable=all --xml -I include/ src/ 2> cppcheck-results.xml

echo "Generate the doc"
make doc

echo "Compress the doc"
cd doc/html/
tar cvzf doc.tar.gz *
mv doc.tar.gz ../../
