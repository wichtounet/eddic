export CCACHE_DIR=/var/tmp/ccache/
cmake -DCMAKE_CXX_COMPILER=/usr/lib/ccache/bin/clang++ -DCMAKE_C_COMPILER=/usr/lib/ccache/bin/clang .
make

echo "Run the test"
./bin/boosttest--eddic_boost_test --report_level=detailed --report_format=xml --report_sink=test-results.xml

echo "Generate the doc"
make doc

echo "Compress the doc"
cd doc/html/
tar cvzf doc.tar.gz *
mv doc.tar.gz ../../
