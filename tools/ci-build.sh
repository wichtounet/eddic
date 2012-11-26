export CCACHE_DIR=/var/tmp/ccache/
cmake -DCMAKE_CXX_COMPILER=/usr/lib/ccache/bin/g++ -DCMAKE_C_COMPILER=/usr/lib/ccache/bin/gcc -DCMAKE_BUILD_TYPE=Coverage .
make

echo "Run the test suite"
./bin/boosttest--eddic_boost_test --report_level=detailed --report_format=xml --report_sink=test-results.xml

echo "Analyze the source code with cppcheck"
cppcheck -v --platform=unix64 --std=c++11 --enable=all --xml -I include/ src/ 2> cppcheck-results.xml

echo "Analyze the source code with RATS"
rats -w 3 --xml src/ include/ > rats-results.xml

echo "Verify memory usage with Valgrind"
valgrind --xml=yes --xml-file=valgrind-results.xml bin/eddic eddi_samples/asm.eddi

echo "Generate the doc"
make doc

echo "Compress the doc"
cd doc/html/
tar cvzf doc.tar.gz *
mv doc.tar.gz ../../

echo "Send the results to Sonar"
sonar-runner
