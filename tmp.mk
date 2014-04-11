TEST_EXE=bin/boosttest--eddic_boost_test

update_test_list: bin/boosttest--eddic_boost_test
	./$(TEST_EXE) --log_level=test_suite --log_sink=stdout > tests.tmp.log
	bash tools/generate_tests.sh

-include tests.mk