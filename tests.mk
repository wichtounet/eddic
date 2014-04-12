debug_test_bb_entry_exit_count : $(DEBUG_TEST_EXE)
	 @ echo "Run bb_entry_exit_count" > test_reports/test_bb_entry_exit_count.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=bb_entry_exit_count --report_sin=stdout >> test_reports/test_bb_entry_exit_count.log

release_test_bb_entry_exit_count : $(RELEASE_TEST_EXE)
	 @ echo "Run bb_entry_exit_count" > test_reports/test_bb_entry_exit_count.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=bb_entry_exit_count --report_sin=stdout >> test_reports/test_bb_entry_exit_count.log

debug_test_bb_iterators : $(DEBUG_TEST_EXE)
	 @ echo "Run bb_iterators" > test_reports/test_bb_iterators.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=bb_iterators --report_sin=stdout >> test_reports/test_bb_iterators.log

release_test_bb_iterators : $(RELEASE_TEST_EXE)
	 @ echo "Run bb_iterators" > test_reports/test_bb_iterators.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=bb_iterators --report_sin=stdout >> test_reports/test_bb_iterators.log

debug_test_bb_new_bb : $(DEBUG_TEST_EXE)
	 @ echo "Run bb_new_bb" > test_reports/test_bb_new_bb.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=bb_new_bb --report_sin=stdout >> test_reports/test_bb_new_bb.log

release_test_bb_new_bb : $(RELEASE_TEST_EXE)
	 @ echo "Run bb_new_bb" > test_reports/test_bb_new_bb.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=bb_new_bb --report_sin=stdout >> test_reports/test_bb_new_bb.log

debug_test_bb_remove_bb : $(DEBUG_TEST_EXE)
	 @ echo "Run bb_remove_bb" > test_reports/test_bb_remove_bb.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=bb_remove_bb --report_sin=stdout >> test_reports/test_bb_remove_bb.log

release_test_bb_remove_bb : $(RELEASE_TEST_EXE)
	 @ echo "Run bb_remove_bb" > test_reports/test_bb_remove_bb.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=bb_remove_bb --report_sin=stdout >> test_reports/test_bb_remove_bb.log

debug_test_bb_at : $(DEBUG_TEST_EXE)
	 @ echo "Run bb_at" > test_reports/test_bb_at.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=bb_at --report_sin=stdout >> test_reports/test_bb_at.log

release_test_bb_at : $(RELEASE_TEST_EXE)
	 @ echo "Run bb_at" > test_reports/test_bb_at.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=bb_at --report_sin=stdout >> test_reports/test_bb_at.log

debug_test_ApplicationsSuite/applications_hangman : $(DEBUG_TEST_EXE)
	 @ echo "Run ApplicationsSuite/applications_hangman" > test_reports/test_ApplicationsSuite-applications_hangman.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=ApplicationsSuite/applications_hangman --report_sin=stdout >> test_reports/test_ApplicationsSuite-applications_hangman.log

release_test_ApplicationsSuite/applications_hangman : $(RELEASE_TEST_EXE)
	 @ echo "Run ApplicationsSuite/applications_hangman" > test_reports/test_ApplicationsSuite-applications_hangman.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=ApplicationsSuite/applications_hangman --report_sin=stdout >> test_reports/test_ApplicationsSuite-applications_hangman.log

debug_test_SamplesSuite/samples_arrays : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_arrays" > test_reports/test_SamplesSuite-samples_arrays.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_arrays --report_sin=stdout >> test_reports/test_SamplesSuite-samples_arrays.log

release_test_SamplesSuite/samples_arrays : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_arrays" > test_reports/test_SamplesSuite-samples_arrays.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_arrays --report_sin=stdout >> test_reports/test_SamplesSuite-samples_arrays.log

debug_test_SamplesSuite/samples_asm : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_asm" > test_reports/test_SamplesSuite-samples_asm.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_asm --report_sin=stdout >> test_reports/test_SamplesSuite-samples_asm.log

release_test_SamplesSuite/samples_asm : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_asm" > test_reports/test_SamplesSuite-samples_asm.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_asm --report_sin=stdout >> test_reports/test_SamplesSuite-samples_asm.log

debug_test_SamplesSuite/samples_assembly : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_assembly" > test_reports/test_SamplesSuite-samples_assembly.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_assembly --report_sin=stdout >> test_reports/test_SamplesSuite-samples_assembly.log

release_test_SamplesSuite/samples_assembly : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_assembly" > test_reports/test_SamplesSuite-samples_assembly.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_assembly --report_sin=stdout >> test_reports/test_SamplesSuite-samples_assembly.log

debug_test_SamplesSuite/samples_bool : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_bool" > test_reports/test_SamplesSuite-samples_bool.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_bool --report_sin=stdout >> test_reports/test_SamplesSuite-samples_bool.log

release_test_SamplesSuite/samples_bool : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_bool" > test_reports/test_SamplesSuite-samples_bool.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_bool --report_sin=stdout >> test_reports/test_SamplesSuite-samples_bool.log

debug_test_SamplesSuite/samples_compound : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_compound" > test_reports/test_SamplesSuite-samples_compound.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_compound --report_sin=stdout >> test_reports/test_SamplesSuite-samples_compound.log

release_test_SamplesSuite/samples_compound : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_compound" > test_reports/test_SamplesSuite-samples_compound.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_compound --report_sin=stdout >> test_reports/test_SamplesSuite-samples_compound.log

debug_test_SamplesSuite/samples_concat : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_concat" > test_reports/test_SamplesSuite-samples_concat.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_concat --report_sin=stdout >> test_reports/test_SamplesSuite-samples_concat.log

release_test_SamplesSuite/samples_concat : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_concat" > test_reports/test_SamplesSuite-samples_concat.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_concat --report_sin=stdout >> test_reports/test_SamplesSuite-samples_concat.log

debug_test_SamplesSuite/samples_const : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_const" > test_reports/test_SamplesSuite-samples_const.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_const --report_sin=stdout >> test_reports/test_SamplesSuite-samples_const.log

release_test_SamplesSuite/samples_const : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_const" > test_reports/test_SamplesSuite-samples_const.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_const --report_sin=stdout >> test_reports/test_SamplesSuite-samples_const.log

debug_test_SamplesSuite/samples_functions : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_functions" > test_reports/test_SamplesSuite-samples_functions.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_functions --report_sin=stdout >> test_reports/test_SamplesSuite-samples_functions.log

release_test_SamplesSuite/samples_functions : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_functions" > test_reports/test_SamplesSuite-samples_functions.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_functions --report_sin=stdout >> test_reports/test_SamplesSuite-samples_functions.log

debug_test_SamplesSuite/samples_float : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_float" > test_reports/test_SamplesSuite-samples_float.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_float.log

release_test_SamplesSuite/samples_float : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_float" > test_reports/test_SamplesSuite-samples_float.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_float.log

debug_test_SamplesSuite/samples_little_float : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_little_float" > test_reports/test_SamplesSuite-samples_little_float.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_little_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_little_float.log

release_test_SamplesSuite/samples_little_float : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_little_float" > test_reports/test_SamplesSuite-samples_little_float.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_little_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_little_float.log

debug_test_SamplesSuite/samples_casts : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_casts" > test_reports/test_SamplesSuite-samples_casts.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_casts --report_sin=stdout >> test_reports/test_SamplesSuite-samples_casts.log

release_test_SamplesSuite/samples_casts : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_casts" > test_reports/test_SamplesSuite-samples_casts.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_casts --report_sin=stdout >> test_reports/test_SamplesSuite-samples_casts.log

debug_test_SamplesSuite/samples_inc : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_inc" > test_reports/test_SamplesSuite-samples_inc.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_inc --report_sin=stdout >> test_reports/test_SamplesSuite-samples_inc.log

release_test_SamplesSuite/samples_inc : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_inc" > test_reports/test_SamplesSuite-samples_inc.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_inc --report_sin=stdout >> test_reports/test_SamplesSuite-samples_inc.log

debug_test_SamplesSuite/samples_includes : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_includes" > test_reports/test_SamplesSuite-samples_includes.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_includes --report_sin=stdout >> test_reports/test_SamplesSuite-samples_includes.log

release_test_SamplesSuite/samples_includes : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_includes" > test_reports/test_SamplesSuite-samples_includes.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_includes --report_sin=stdout >> test_reports/test_SamplesSuite-samples_includes.log

debug_test_SamplesSuite/samples_optimize : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_optimize" > test_reports/test_SamplesSuite-samples_optimize.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_optimize --report_sin=stdout >> test_reports/test_SamplesSuite-samples_optimize.log

release_test_SamplesSuite/samples_optimize : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_optimize" > test_reports/test_SamplesSuite-samples_optimize.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_optimize --report_sin=stdout >> test_reports/test_SamplesSuite-samples_optimize.log

debug_test_SamplesSuite/samples_problem : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_problem" > test_reports/test_SamplesSuite-samples_problem.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_problem --report_sin=stdout >> test_reports/test_SamplesSuite-samples_problem.log

release_test_SamplesSuite/samples_problem : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_problem" > test_reports/test_SamplesSuite-samples_problem.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_problem --report_sin=stdout >> test_reports/test_SamplesSuite-samples_problem.log

debug_test_SamplesSuite/samples_identifiers : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_identifiers" > test_reports/test_SamplesSuite-samples_identifiers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_identifiers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_identifiers.log

release_test_SamplesSuite/samples_identifiers : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_identifiers" > test_reports/test_SamplesSuite-samples_identifiers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_identifiers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_identifiers.log

debug_test_SamplesSuite/samples_registers : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_registers" > test_reports/test_SamplesSuite-samples_registers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_registers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_registers.log

release_test_SamplesSuite/samples_registers : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_registers" > test_reports/test_SamplesSuite-samples_registers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_registers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_registers.log

debug_test_SamplesSuite/samples_structures : $(DEBUG_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_structures" > test_reports/test_SamplesSuite-samples_structures.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SamplesSuite/samples_structures --report_sin=stdout >> test_reports/test_SamplesSuite-samples_structures.log

release_test_SamplesSuite/samples_structures : $(RELEASE_TEST_EXE)
	 @ echo "Run SamplesSuite/samples_structures" > test_reports/test_SamplesSuite-samples_structures.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SamplesSuite/samples_structures --report_sin=stdout >> test_reports/test_SamplesSuite-samples_structures.log

debug_test_SpecificSuite/addressof : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/addressof" > test_reports/test_SpecificSuite-addressof.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/addressof --report_sin=stdout >> test_reports/test_SpecificSuite-addressof.log

release_test_SpecificSuite/addressof : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/addressof" > test_reports/test_SpecificSuite-addressof.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/addressof --report_sin=stdout >> test_reports/test_SpecificSuite-addressof.log

debug_test_SpecificSuite/array_foreach_local : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_local" > test_reports/test_SpecificSuite-array_foreach_local.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/array_foreach_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_local.log

release_test_SpecificSuite/array_foreach_local : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_local" > test_reports/test_SpecificSuite-array_foreach_local.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/array_foreach_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_local.log

debug_test_SpecificSuite/array_foreach_global : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_global" > test_reports/test_SpecificSuite-array_foreach_global.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/array_foreach_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_global.log

release_test_SpecificSuite/array_foreach_global : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_global" > test_reports/test_SpecificSuite-array_foreach_global.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/array_foreach_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_global.log

debug_test_SpecificSuite/array_foreach_param_local : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_local" > test_reports/test_SpecificSuite-array_foreach_param_local.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_local.log

release_test_SpecificSuite/array_foreach_param_local : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_local" > test_reports/test_SpecificSuite-array_foreach_param_local.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_local.log

debug_test_SpecificSuite/array_foreach_param_global : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_global" > test_reports/test_SpecificSuite-array_foreach_param_global.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_global.log

release_test_SpecificSuite/array_foreach_param_global : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_global" > test_reports/test_SpecificSuite-array_foreach_param_global.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_global.log

debug_test_SpecificSuite/array_foreach_param_param : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_param" > test_reports/test_SpecificSuite-array_foreach_param_param.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_param --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_param.log

release_test_SpecificSuite/array_foreach_param_param : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_param" > test_reports/test_SpecificSuite-array_foreach_param_param.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/array_foreach_param_param --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_param.log

debug_test_SpecificSuite/arrays_in_struct : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/arrays_in_struct" > test_reports/test_SpecificSuite-arrays_in_struct.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-arrays_in_struct.log

release_test_SpecificSuite/arrays_in_struct : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/arrays_in_struct" > test_reports/test_SpecificSuite-arrays_in_struct.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-arrays_in_struct.log

debug_test_SpecificSuite/char_type : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/char_type" > test_reports/test_SpecificSuite-char_type.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/char_type --report_sin=stdout >> test_reports/test_SpecificSuite-char_type.log

release_test_SpecificSuite/char_type : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/char_type" > test_reports/test_SpecificSuite-char_type.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/char_type --report_sin=stdout >> test_reports/test_SpecificSuite-char_type.log

debug_test_SpecificSuite/char_at : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/char_at" > test_reports/test_SpecificSuite-char_at.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/char_at --report_sin=stdout >> test_reports/test_SpecificSuite-char_at.log

release_test_SpecificSuite/char_at : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/char_at" > test_reports/test_SpecificSuite-char_at.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/char_at --report_sin=stdout >> test_reports/test_SpecificSuite-char_at.log

debug_test_SpecificSuite/ctor_dtor_heap : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_heap" > test_reports/test_SpecificSuite-ctor_dtor_heap.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/ctor_dtor_heap --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_heap.log

release_test_SpecificSuite/ctor_dtor_heap : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_heap" > test_reports/test_SpecificSuite-ctor_dtor_heap.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/ctor_dtor_heap --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_heap.log

debug_test_SpecificSuite/ctor_dtor_stack : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_stack" > test_reports/test_SpecificSuite-ctor_dtor_stack.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/ctor_dtor_stack --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_stack.log

release_test_SpecificSuite/ctor_dtor_stack : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_stack" > test_reports/test_SpecificSuite-ctor_dtor_stack.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/ctor_dtor_stack --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_stack.log

debug_test_SpecificSuite/copy_constructors : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/copy_constructors" > test_reports/test_SpecificSuite-copy_constructors.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/copy_constructors --report_sin=stdout >> test_reports/test_SpecificSuite-copy_constructors.log

release_test_SpecificSuite/copy_constructors : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/copy_constructors" > test_reports/test_SpecificSuite-copy_constructors.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/copy_constructors --report_sin=stdout >> test_reports/test_SpecificSuite-copy_constructors.log

debug_test_SpecificSuite/casts : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/casts" > test_reports/test_SpecificSuite-casts.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/casts --report_sin=stdout >> test_reports/test_SpecificSuite-casts.log

release_test_SpecificSuite/casts : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/casts" > test_reports/test_SpecificSuite-casts.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/casts --report_sin=stdout >> test_reports/test_SpecificSuite-casts.log

debug_test_SpecificSuite/compound : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/compound" > test_reports/test_SpecificSuite-compound.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/compound --report_sin=stdout >> test_reports/test_SpecificSuite-compound.log

release_test_SpecificSuite/compound : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/compound" > test_reports/test_SpecificSuite-compound.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/compound --report_sin=stdout >> test_reports/test_SpecificSuite-compound.log

debug_test_SpecificSuite/delete_any : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/delete_any" > test_reports/test_SpecificSuite-delete_any.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/delete_any --report_sin=stdout >> test_reports/test_SpecificSuite-delete_any.log

release_test_SpecificSuite/delete_any : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/delete_any" > test_reports/test_SpecificSuite-delete_any.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/delete_any --report_sin=stdout >> test_reports/test_SpecificSuite-delete_any.log

debug_test_SpecificSuite/if_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/if_" > test_reports/test_SpecificSuite-if_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/if_ --report_sin=stdout >> test_reports/test_SpecificSuite-if_.log

release_test_SpecificSuite/if_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/if_" > test_reports/test_SpecificSuite-if_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/if_ --report_sin=stdout >> test_reports/test_SpecificSuite-if_.log

debug_test_SpecificSuite/includes : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/includes" > test_reports/test_SpecificSuite-includes.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/includes --report_sin=stdout >> test_reports/test_SpecificSuite-includes.log

release_test_SpecificSuite/includes : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/includes" > test_reports/test_SpecificSuite-includes.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/includes --report_sin=stdout >> test_reports/test_SpecificSuite-includes.log

debug_test_SpecificSuite/int_arrays : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/int_arrays" > test_reports/test_SpecificSuite-int_arrays.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/int_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-int_arrays.log

release_test_SpecificSuite/int_arrays : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/int_arrays" > test_reports/test_SpecificSuite-int_arrays.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/int_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-int_arrays.log

debug_test_SpecificSuite/string_arrays : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/string_arrays" > test_reports/test_SpecificSuite-string_arrays.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/string_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-string_arrays.log

release_test_SpecificSuite/string_arrays : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/string_arrays" > test_reports/test_SpecificSuite-string_arrays.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/string_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-string_arrays.log

debug_test_SpecificSuite/string_foreach : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/string_foreach" > test_reports/test_SpecificSuite-string_foreach.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/string_foreach --report_sin=stdout >> test_reports/test_SpecificSuite-string_foreach.log

release_test_SpecificSuite/string_foreach : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/string_foreach" > test_reports/test_SpecificSuite-string_foreach.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/string_foreach --report_sin=stdout >> test_reports/test_SpecificSuite-string_foreach.log

debug_test_SpecificSuite/string_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/string_pointers" > test_reports/test_SpecificSuite-string_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/string_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-string_pointers.log

release_test_SpecificSuite/string_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/string_pointers" > test_reports/test_SpecificSuite-string_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/string_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-string_pointers.log

debug_test_SpecificSuite/int_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/int_pointers" > test_reports/test_SpecificSuite-int_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/int_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-int_pointers.log

release_test_SpecificSuite/int_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/int_pointers" > test_reports/test_SpecificSuite-int_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/int_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-int_pointers.log

debug_test_SpecificSuite/bool_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/bool_pointers" > test_reports/test_SpecificSuite-bool_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/bool_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-bool_pointers.log

release_test_SpecificSuite/bool_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/bool_pointers" > test_reports/test_SpecificSuite-bool_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/bool_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-bool_pointers.log

debug_test_SpecificSuite/cmove : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/cmove" > test_reports/test_SpecificSuite-cmove.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/cmove --report_sin=stdout >> test_reports/test_SpecificSuite-cmove.log

release_test_SpecificSuite/cmove : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/cmove" > test_reports/test_SpecificSuite-cmove.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/cmove --report_sin=stdout >> test_reports/test_SpecificSuite-cmove.log

debug_test_SpecificSuite/dynamic : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic" > test_reports/test_SpecificSuite-dynamic.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/dynamic --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic.log

release_test_SpecificSuite/dynamic : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic" > test_reports/test_SpecificSuite-dynamic.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/dynamic --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic.log

debug_test_SpecificSuite/dynamic_arrays_in_struct : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_arrays_in_struct" > test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/dynamic_arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log

release_test_SpecificSuite/dynamic_arrays_in_struct : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_arrays_in_struct" > test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/dynamic_arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log

debug_test_SpecificSuite/dynamic_struct : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_struct" > test_reports/test_SpecificSuite-dynamic_struct.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/dynamic_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_struct.log

release_test_SpecificSuite/dynamic_struct : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_struct" > test_reports/test_SpecificSuite-dynamic_struct.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/dynamic_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_struct.log

debug_test_SpecificSuite/float_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/float_pointers" > test_reports/test_SpecificSuite-float_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/float_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-float_pointers.log

release_test_SpecificSuite/float_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/float_pointers" > test_reports/test_SpecificSuite-float_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/float_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-float_pointers.log

debug_test_SpecificSuite/struct_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_pointers" > test_reports/test_SpecificSuite-struct_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/struct_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_pointers.log

release_test_SpecificSuite/struct_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_pointers" > test_reports/test_SpecificSuite-struct_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/struct_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_pointers.log

debug_test_SpecificSuite/member_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/member_pointers" > test_reports/test_SpecificSuite-member_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-member_pointers.log

release_test_SpecificSuite/member_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/member_pointers" > test_reports/test_SpecificSuite-member_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-member_pointers.log

debug_test_SpecificSuite/member_function_calls : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/member_function_calls" > test_reports/test_SpecificSuite-member_function_calls.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/member_function_calls --report_sin=stdout >> test_reports/test_SpecificSuite-member_function_calls.log

release_test_SpecificSuite/member_function_calls : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/member_function_calls" > test_reports/test_SpecificSuite-member_function_calls.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/member_function_calls --report_sin=stdout >> test_reports/test_SpecificSuite-member_function_calls.log

debug_test_SpecificSuite/member_functions : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions" > test_reports/test_SpecificSuite-member_functions.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/member_functions --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions.log

release_test_SpecificSuite/member_functions : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions" > test_reports/test_SpecificSuite-member_functions.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/member_functions --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions.log

debug_test_SpecificSuite/member_functions_param_stack : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions_param_stack" > test_reports/test_SpecificSuite-member_functions_param_stack.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/member_functions_param_stack --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions_param_stack.log

release_test_SpecificSuite/member_functions_param_stack : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions_param_stack" > test_reports/test_SpecificSuite-member_functions_param_stack.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/member_functions_param_stack --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions_param_stack.log

debug_test_SpecificSuite/memory : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/memory" > test_reports/test_SpecificSuite-memory.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/memory --report_sin=stdout >> test_reports/test_SpecificSuite-memory.log

release_test_SpecificSuite/memory : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/memory" > test_reports/test_SpecificSuite-memory.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/memory --report_sin=stdout >> test_reports/test_SpecificSuite-memory.log

debug_test_SpecificSuite/pass_member_by_value : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/pass_member_by_value" > test_reports/test_SpecificSuite-pass_member_by_value.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/pass_member_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-pass_member_by_value.log

release_test_SpecificSuite/pass_member_by_value : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/pass_member_by_value" > test_reports/test_SpecificSuite-pass_member_by_value.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/pass_member_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-pass_member_by_value.log

debug_test_SpecificSuite/ternary : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/ternary" > test_reports/test_SpecificSuite-ternary.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/ternary --report_sin=stdout >> test_reports/test_SpecificSuite-ternary.log

release_test_SpecificSuite/ternary : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/ternary" > test_reports/test_SpecificSuite-ternary.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/ternary --report_sin=stdout >> test_reports/test_SpecificSuite-ternary.log

debug_test_SpecificSuite/while_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/while_" > test_reports/test_SpecificSuite-while_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/while_ --report_sin=stdout >> test_reports/test_SpecificSuite-while_.log

release_test_SpecificSuite/while_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/while_" > test_reports/test_SpecificSuite-while_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/while_ --report_sin=stdout >> test_reports/test_SpecificSuite-while_.log

debug_test_SpecificSuite/do_while_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/do_while_" > test_reports/test_SpecificSuite-do_while_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/do_while_ --report_sin=stdout >> test_reports/test_SpecificSuite-do_while_.log

release_test_SpecificSuite/do_while_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/do_while_" > test_reports/test_SpecificSuite-do_while_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/do_while_ --report_sin=stdout >> test_reports/test_SpecificSuite-do_while_.log

debug_test_SpecificSuite/defaults : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/defaults" > test_reports/test_SpecificSuite-defaults.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/defaults --report_sin=stdout >> test_reports/test_SpecificSuite-defaults.log

release_test_SpecificSuite/defaults : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/defaults" > test_reports/test_SpecificSuite-defaults.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/defaults --report_sin=stdout >> test_reports/test_SpecificSuite-defaults.log

debug_test_SpecificSuite/float_1 : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/float_1" > test_reports/test_SpecificSuite-float_1.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/float_1 --report_sin=stdout >> test_reports/test_SpecificSuite-float_1.log

release_test_SpecificSuite/float_1 : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/float_1" > test_reports/test_SpecificSuite-float_1.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/float_1 --report_sin=stdout >> test_reports/test_SpecificSuite-float_1.log

debug_test_SpecificSuite/float_2 : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/float_2" > test_reports/test_SpecificSuite-float_2.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/float_2 --report_sin=stdout >> test_reports/test_SpecificSuite-float_2.log

release_test_SpecificSuite/float_2 : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/float_2" > test_reports/test_SpecificSuite-float_2.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/float_2 --report_sin=stdout >> test_reports/test_SpecificSuite-float_2.log

debug_test_SpecificSuite/for_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/for_" > test_reports/test_SpecificSuite-for_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/for_ --report_sin=stdout >> test_reports/test_SpecificSuite-for_.log

release_test_SpecificSuite/for_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/for_" > test_reports/test_SpecificSuite-for_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/for_ --report_sin=stdout >> test_reports/test_SpecificSuite-for_.log

debug_test_SpecificSuite/foreach_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/foreach_" > test_reports/test_SpecificSuite-foreach_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/foreach_ --report_sin=stdout >> test_reports/test_SpecificSuite-foreach_.log

release_test_SpecificSuite/foreach_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/foreach_" > test_reports/test_SpecificSuite-foreach_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/foreach_ --report_sin=stdout >> test_reports/test_SpecificSuite-foreach_.log

debug_test_SpecificSuite/globals_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/globals_" > test_reports/test_SpecificSuite-globals_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/globals_ --report_sin=stdout >> test_reports/test_SpecificSuite-globals_.log

release_test_SpecificSuite/globals_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/globals_" > test_reports/test_SpecificSuite-globals_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/globals_ --report_sin=stdout >> test_reports/test_SpecificSuite-globals_.log

debug_test_SpecificSuite/inc : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/inc" > test_reports/test_SpecificSuite-inc.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/inc --report_sin=stdout >> test_reports/test_SpecificSuite-inc.log

release_test_SpecificSuite/inc : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/inc" > test_reports/test_SpecificSuite-inc.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/inc --report_sin=stdout >> test_reports/test_SpecificSuite-inc.log

debug_test_SpecificSuite/void_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/void_" > test_reports/test_SpecificSuite-void_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/void_ --report_sin=stdout >> test_reports/test_SpecificSuite-void_.log

release_test_SpecificSuite/void_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/void_" > test_reports/test_SpecificSuite-void_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/void_ --report_sin=stdout >> test_reports/test_SpecificSuite-void_.log

debug_test_SpecificSuite/return_string : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/return_string" > test_reports/test_SpecificSuite-return_string.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/return_string --report_sin=stdout >> test_reports/test_SpecificSuite-return_string.log

release_test_SpecificSuite/return_string : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/return_string" > test_reports/test_SpecificSuite-return_string.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/return_string --report_sin=stdout >> test_reports/test_SpecificSuite-return_string.log

debug_test_SpecificSuite/return_by_value : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/return_by_value" > test_reports/test_SpecificSuite-return_by_value.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/return_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-return_by_value.log

release_test_SpecificSuite/return_by_value : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/return_by_value" > test_reports/test_SpecificSuite-return_by_value.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/return_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-return_by_value.log

debug_test_SpecificSuite/return_int : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/return_int" > test_reports/test_SpecificSuite-return_int.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/return_int --report_sin=stdout >> test_reports/test_SpecificSuite-return_int.log

release_test_SpecificSuite/return_int : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/return_int" > test_reports/test_SpecificSuite-return_int.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/return_int --report_sin=stdout >> test_reports/test_SpecificSuite-return_int.log

debug_test_SpecificSuite/return_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/return_pointers" > test_reports/test_SpecificSuite-return_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/return_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-return_pointers.log

release_test_SpecificSuite/return_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/return_pointers" > test_reports/test_SpecificSuite-return_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/return_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-return_pointers.log

debug_test_SpecificSuite/pointer_arrays : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/pointer_arrays" > test_reports/test_SpecificSuite-pointer_arrays.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/pointer_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-pointer_arrays.log

release_test_SpecificSuite/pointer_arrays : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/pointer_arrays" > test_reports/test_SpecificSuite-pointer_arrays.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/pointer_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-pointer_arrays.log

debug_test_SpecificSuite/recursive_functions : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/recursive_functions" > test_reports/test_SpecificSuite-recursive_functions.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/recursive_functions --report_sin=stdout >> test_reports/test_SpecificSuite-recursive_functions.log

release_test_SpecificSuite/recursive_functions : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/recursive_functions" > test_reports/test_SpecificSuite-recursive_functions.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/recursive_functions --report_sin=stdout >> test_reports/test_SpecificSuite-recursive_functions.log

debug_test_SpecificSuite/single_inheritance : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/single_inheritance" > test_reports/test_SpecificSuite-single_inheritance.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/single_inheritance --report_sin=stdout >> test_reports/test_SpecificSuite-single_inheritance.log

release_test_SpecificSuite/single_inheritance : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/single_inheritance" > test_reports/test_SpecificSuite-single_inheritance.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/single_inheritance --report_sin=stdout >> test_reports/test_SpecificSuite-single_inheritance.log

debug_test_SpecificSuite/math : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/math" > test_reports/test_SpecificSuite-math.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/math --report_sin=stdout >> test_reports/test_SpecificSuite-math.log

release_test_SpecificSuite/math : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/math" > test_reports/test_SpecificSuite-math.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/math --report_sin=stdout >> test_reports/test_SpecificSuite-math.log

debug_test_SpecificSuite/builtin : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/builtin" > test_reports/test_SpecificSuite-builtin.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/builtin --report_sin=stdout >> test_reports/test_SpecificSuite-builtin.log

release_test_SpecificSuite/builtin : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/builtin" > test_reports/test_SpecificSuite-builtin.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/builtin --report_sin=stdout >> test_reports/test_SpecificSuite-builtin.log

debug_test_SpecificSuite/assign_value : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/assign_value" > test_reports/test_SpecificSuite-assign_value.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/assign_value --report_sin=stdout >> test_reports/test_SpecificSuite-assign_value.log

release_test_SpecificSuite/assign_value : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/assign_value" > test_reports/test_SpecificSuite-assign_value.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/assign_value --report_sin=stdout >> test_reports/test_SpecificSuite-assign_value.log

debug_test_SpecificSuite/println : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/println" > test_reports/test_SpecificSuite-println.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/println --report_sin=stdout >> test_reports/test_SpecificSuite-println.log

release_test_SpecificSuite/println : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/println" > test_reports/test_SpecificSuite-println.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/println --report_sin=stdout >> test_reports/test_SpecificSuite-println.log

debug_test_SpecificSuite/prints : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/prints" > test_reports/test_SpecificSuite-prints.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/prints --report_sin=stdout >> test_reports/test_SpecificSuite-prints.log

release_test_SpecificSuite/prints : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/prints" > test_reports/test_SpecificSuite-prints.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/prints --report_sin=stdout >> test_reports/test_SpecificSuite-prints.log

debug_test_SpecificSuite/structures : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/structures" > test_reports/test_SpecificSuite-structures.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/structures --report_sin=stdout >> test_reports/test_SpecificSuite-structures.log

release_test_SpecificSuite/structures : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/structures" > test_reports/test_SpecificSuite-structures.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/structures --report_sin=stdout >> test_reports/test_SpecificSuite-structures.log

debug_test_SpecificSuite/struct_member_pointers : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_member_pointers" > test_reports/test_SpecificSuite-struct_member_pointers.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/struct_member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_member_pointers.log

release_test_SpecificSuite/struct_member_pointers : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_member_pointers" > test_reports/test_SpecificSuite-struct_member_pointers.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/struct_member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_member_pointers.log

debug_test_SpecificSuite/struct_array : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_array" > test_reports/test_SpecificSuite-struct_array.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/struct_array --report_sin=stdout >> test_reports/test_SpecificSuite-struct_array.log

release_test_SpecificSuite/struct_array : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/struct_array" > test_reports/test_SpecificSuite-struct_array.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/struct_array --report_sin=stdout >> test_reports/test_SpecificSuite-struct_array.log

debug_test_SpecificSuite/switch_ : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/switch_" > test_reports/test_SpecificSuite-switch_.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/switch_ --report_sin=stdout >> test_reports/test_SpecificSuite-switch_.log

release_test_SpecificSuite/switch_ : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/switch_" > test_reports/test_SpecificSuite-switch_.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/switch_ --report_sin=stdout >> test_reports/test_SpecificSuite-switch_.log

debug_test_SpecificSuite/switch_string : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/switch_string" > test_reports/test_SpecificSuite-switch_string.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/switch_string --report_sin=stdout >> test_reports/test_SpecificSuite-switch_string.log

release_test_SpecificSuite/switch_string : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/switch_string" > test_reports/test_SpecificSuite-switch_string.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/switch_string --report_sin=stdout >> test_reports/test_SpecificSuite-switch_string.log

debug_test_SpecificSuite/nested : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/nested" > test_reports/test_SpecificSuite-nested.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/nested --report_sin=stdout >> test_reports/test_SpecificSuite-nested.log

release_test_SpecificSuite/nested : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/nested" > test_reports/test_SpecificSuite-nested.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/nested --report_sin=stdout >> test_reports/test_SpecificSuite-nested.log

debug_test_SpecificSuite/args : $(DEBUG_TEST_EXE)
	 @ echo "Run SpecificSuite/args" > test_reports/test_SpecificSuite-args.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=SpecificSuite/args --report_sin=stdout >> test_reports/test_SpecificSuite-args.log

release_test_SpecificSuite/args : $(RELEASE_TEST_EXE)
	 @ echo "Run SpecificSuite/args" > test_reports/test_SpecificSuite-args.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=SpecificSuite/args --report_sin=stdout >> test_reports/test_SpecificSuite-args.log

debug_test_TemplateSuite/class_templates : $(DEBUG_TEST_EXE)
	 @ echo "Run TemplateSuite/class_templates" > test_reports/test_TemplateSuite-class_templates.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=TemplateSuite/class_templates --report_sin=stdout >> test_reports/test_TemplateSuite-class_templates.log

release_test_TemplateSuite/class_templates : $(RELEASE_TEST_EXE)
	 @ echo "Run TemplateSuite/class_templates" > test_reports/test_TemplateSuite-class_templates.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=TemplateSuite/class_templates --report_sin=stdout >> test_reports/test_TemplateSuite-class_templates.log

debug_test_TemplateSuite/function_templates : $(DEBUG_TEST_EXE)
	 @ echo "Run TemplateSuite/function_templates" > test_reports/test_TemplateSuite-function_templates.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=TemplateSuite/function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-function_templates.log

release_test_TemplateSuite/function_templates : $(RELEASE_TEST_EXE)
	 @ echo "Run TemplateSuite/function_templates" > test_reports/test_TemplateSuite-function_templates.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=TemplateSuite/function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-function_templates.log

debug_test_TemplateSuite/member_function_templates : $(DEBUG_TEST_EXE)
	 @ echo "Run TemplateSuite/member_function_templates" > test_reports/test_TemplateSuite-member_function_templates.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=TemplateSuite/member_function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-member_function_templates.log

release_test_TemplateSuite/member_function_templates : $(RELEASE_TEST_EXE)
	 @ echo "Run TemplateSuite/member_function_templates" > test_reports/test_TemplateSuite-member_function_templates.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=TemplateSuite/member_function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-member_function_templates.log

debug_test_CompilationErrorsSuite/params_assign : $(DEBUG_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/params_assign" > test_reports/test_CompilationErrorsSuite-params_assign.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=CompilationErrorsSuite/params_assign --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-params_assign.log

release_test_CompilationErrorsSuite/params_assign : $(RELEASE_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/params_assign" > test_reports/test_CompilationErrorsSuite-params_assign.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=CompilationErrorsSuite/params_assign --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-params_assign.log

debug_test_CompilationErrorsSuite/wrong_print : $(DEBUG_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/wrong_print" > test_reports/test_CompilationErrorsSuite-wrong_print.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=CompilationErrorsSuite/wrong_print --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-wrong_print.log

release_test_CompilationErrorsSuite/wrong_print : $(RELEASE_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/wrong_print" > test_reports/test_CompilationErrorsSuite-wrong_print.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=CompilationErrorsSuite/wrong_print --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-wrong_print.log

debug_test_CompilationErrorsSuite/invalid_inheritance : $(DEBUG_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/invalid_inheritance" > test_reports/test_CompilationErrorsSuite-invalid_inheritance.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=CompilationErrorsSuite/invalid_inheritance --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-invalid_inheritance.log

release_test_CompilationErrorsSuite/invalid_inheritance : $(RELEASE_TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/invalid_inheritance" > test_reports/test_CompilationErrorsSuite-invalid_inheritance.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=CompilationErrorsSuite/invalid_inheritance --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-invalid_inheritance.log

debug_test_StandardLibSuite/std_lib_arrays_sum : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_arrays_sum" > test_reports/test_StandardLibSuite-std_lib_arrays_sum.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_arrays_sum --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_arrays_sum.log

release_test_StandardLibSuite/std_lib_arrays_sum : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_arrays_sum" > test_reports/test_StandardLibSuite-std_lib_arrays_sum.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_arrays_sum --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_arrays_sum.log

debug_test_StandardLibSuite/std_lib_math_min : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_min" > test_reports/test_StandardLibSuite-std_lib_math_min.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_min --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_min.log

release_test_StandardLibSuite/std_lib_math_min : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_min" > test_reports/test_StandardLibSuite-std_lib_math_min.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_min --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_min.log

debug_test_StandardLibSuite/std_lib_math_max : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_max" > test_reports/test_StandardLibSuite-std_lib_math_max.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_max --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_max.log

release_test_StandardLibSuite/std_lib_math_max : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_max" > test_reports/test_StandardLibSuite-std_lib_math_max.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_max --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_max.log

debug_test_StandardLibSuite/std_lib_math_factorial : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_factorial" > test_reports/test_StandardLibSuite-std_lib_math_factorial.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_factorial --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_factorial.log

release_test_StandardLibSuite/std_lib_math_factorial : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_factorial" > test_reports/test_StandardLibSuite-std_lib_math_factorial.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_factorial --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_factorial.log

debug_test_StandardLibSuite/std_lib_math_pow : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_pow" > test_reports/test_StandardLibSuite-std_lib_math_pow.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_pow --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_pow.log

release_test_StandardLibSuite/std_lib_math_pow : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_pow" > test_reports/test_StandardLibSuite-std_lib_math_pow.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_math_pow --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_pow.log

debug_test_StandardLibSuite/std_lib_str_equals : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_str_equals" > test_reports/test_StandardLibSuite-std_lib_str_equals.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_str_equals --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_str_equals.log

release_test_StandardLibSuite/std_lib_str_equals : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_str_equals" > test_reports/test_StandardLibSuite-std_lib_str_equals.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_str_equals --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_str_equals.log

debug_test_StandardLibSuite/std_lib_string : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string" > test_reports/test_StandardLibSuite-std_lib_string.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_string --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string.log

release_test_StandardLibSuite/std_lib_string : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string" > test_reports/test_StandardLibSuite-std_lib_string.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_string --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string.log

debug_test_StandardLibSuite/std_lib_string_concat : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat" > test_reports/test_StandardLibSuite-std_lib_string_concat.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat.log

release_test_StandardLibSuite/std_lib_string_concat : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat" > test_reports/test_StandardLibSuite-std_lib_string_concat.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat.log

debug_test_StandardLibSuite/std_lib_string_concat_int : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat_int" > test_reports/test_StandardLibSuite-std_lib_string_concat_int.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat_int --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat_int.log

release_test_StandardLibSuite/std_lib_string_concat_int : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat_int" > test_reports/test_StandardLibSuite-std_lib_string_concat_int.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat_int --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat_int.log

debug_test_StandardLibSuite/std_linked_list : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_linked_list" > test_reports/test_StandardLibSuite-std_linked_list.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_linked_list --report_sin=stdout >> test_reports/test_StandardLibSuite-std_linked_list.log

release_test_StandardLibSuite/std_linked_list : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_linked_list" > test_reports/test_StandardLibSuite-std_linked_list.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_linked_list --report_sin=stdout >> test_reports/test_StandardLibSuite-std_linked_list.log

debug_test_StandardLibSuite/std_vector : $(DEBUG_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_vector" > test_reports/test_StandardLibSuite-std_vector.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=StandardLibSuite/std_vector --report_sin=stdout >> test_reports/test_StandardLibSuite-std_vector.log

release_test_StandardLibSuite/std_vector : $(RELEASE_TEST_EXE)
	 @ echo "Run StandardLibSuite/std_vector" > test_reports/test_StandardLibSuite-std_vector.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=StandardLibSuite/std_vector --report_sin=stdout >> test_reports/test_StandardLibSuite-std_vector.log

debug_test_BugFixesSuite/while_bug : $(DEBUG_TEST_EXE)
	 @ echo "Run BugFixesSuite/while_bug" > test_reports/test_BugFixesSuite-while_bug.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=BugFixesSuite/while_bug --report_sin=stdout >> test_reports/test_BugFixesSuite-while_bug.log

release_test_BugFixesSuite/while_bug : $(RELEASE_TEST_EXE)
	 @ echo "Run BugFixesSuite/while_bug" > test_reports/test_BugFixesSuite-while_bug.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=BugFixesSuite/while_bug --report_sin=stdout >> test_reports/test_BugFixesSuite-while_bug.log

debug_test_OptimizationSuite/parameter_propagation : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/parameter_propagation" > test_reports/test_OptimizationSuite-parameter_propagation.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/parameter_propagation --report_sin=stdout >> test_reports/test_OptimizationSuite-parameter_propagation.log

release_test_OptimizationSuite/parameter_propagation : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/parameter_propagation" > test_reports/test_OptimizationSuite-parameter_propagation.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/parameter_propagation --report_sin=stdout >> test_reports/test_OptimizationSuite-parameter_propagation.log

debug_test_OptimizationSuite/global_cp : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cp" > test_reports/test_OptimizationSuite-global_cp.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/global_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cp.log

release_test_OptimizationSuite/global_cp : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cp" > test_reports/test_OptimizationSuite-global_cp.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/global_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cp.log

debug_test_OptimizationSuite/global_offset_cp : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_offset_cp" > test_reports/test_OptimizationSuite-global_offset_cp.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/global_offset_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_offset_cp.log

release_test_OptimizationSuite/global_offset_cp : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_offset_cp" > test_reports/test_OptimizationSuite-global_offset_cp.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/global_offset_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_offset_cp.log

debug_test_OptimizationSuite/remove_empty_functions : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_functions" > test_reports/test_OptimizationSuite-remove_empty_functions.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/remove_empty_functions --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_functions.log

release_test_OptimizationSuite/remove_empty_functions : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_functions" > test_reports/test_OptimizationSuite-remove_empty_functions.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/remove_empty_functions --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_functions.log

debug_test_OptimizationSuite/remove_empty_loops : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_loops" > test_reports/test_OptimizationSuite-remove_empty_loops.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/remove_empty_loops --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_loops.log

release_test_OptimizationSuite/remove_empty_loops : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_loops" > test_reports/test_OptimizationSuite-remove_empty_loops.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/remove_empty_loops --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_loops.log

debug_test_OptimizationSuite/invariant_code_motion : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/invariant_code_motion" > test_reports/test_OptimizationSuite-invariant_code_motion.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/invariant_code_motion --report_sin=stdout >> test_reports/test_OptimizationSuite-invariant_code_motion.log

release_test_OptimizationSuite/invariant_code_motion : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/invariant_code_motion" > test_reports/test_OptimizationSuite-invariant_code_motion.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/invariant_code_motion --report_sin=stdout >> test_reports/test_OptimizationSuite-invariant_code_motion.log

debug_test_OptimizationSuite/complete_loop_peeling : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling" > test_reports/test_OptimizationSuite-complete_loop_peeling.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling.log

release_test_OptimizationSuite/complete_loop_peeling : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling" > test_reports/test_OptimizationSuite-complete_loop_peeling.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling.log

debug_test_OptimizationSuite/complete_loop_peeling_2 : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling_2" > test_reports/test_OptimizationSuite-complete_loop_peeling_2.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling_2 --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling_2.log

release_test_OptimizationSuite/complete_loop_peeling_2 : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling_2" > test_reports/test_OptimizationSuite-complete_loop_peeling_2.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling_2 --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling_2.log

debug_test_OptimizationSuite/loop_unrolling : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unrolling" > test_reports/test_OptimizationSuite-loop_unrolling.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/loop_unrolling --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unrolling.log

release_test_OptimizationSuite/loop_unrolling : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unrolling" > test_reports/test_OptimizationSuite-loop_unrolling.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/loop_unrolling --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unrolling.log

debug_test_OptimizationSuite/loop_unswitching : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unswitching" > test_reports/test_OptimizationSuite-loop_unswitching.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/loop_unswitching --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unswitching.log

release_test_OptimizationSuite/loop_unswitching : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unswitching" > test_reports/test_OptimizationSuite-loop_unswitching.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/loop_unswitching --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unswitching.log

debug_test_OptimizationSuite/global_cse : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cse" > test_reports/test_OptimizationSuite-global_cse.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/global_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cse.log

release_test_OptimizationSuite/global_cse : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cse" > test_reports/test_OptimizationSuite-global_cse.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/global_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cse.log

debug_test_OptimizationSuite/local_cse : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/local_cse" > test_reports/test_OptimizationSuite-local_cse.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/local_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-local_cse.log

release_test_OptimizationSuite/local_cse : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/local_cse" > test_reports/test_OptimizationSuite-local_cse.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/local_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-local_cse.log

debug_test_OptimizationSuite/cmov_opt : $(DEBUG_TEST_EXE)
	 @ echo "Run OptimizationSuite/cmov_opt" > test_reports/test_OptimizationSuite-cmov_opt.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=OptimizationSuite/cmov_opt --report_sin=stdout >> test_reports/test_OptimizationSuite-cmov_opt.log

release_test_OptimizationSuite/cmov_opt : $(RELEASE_TEST_EXE)
	 @ echo "Run OptimizationSuite/cmov_opt" > test_reports/test_OptimizationSuite-cmov_opt.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=OptimizationSuite/cmov_opt --report_sin=stdout >> test_reports/test_OptimizationSuite-cmov_opt.log

debug_test_toString : $(DEBUG_TEST_EXE)
	 @ echo "Run toString" > test_reports/test_toString.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=toString --report_sin=stdout >> test_reports/test_toString.log

release_test_toString : $(RELEASE_TEST_EXE)
	 @ echo "Run toString" > test_reports/test_toString.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=toString --report_sin=stdout >> test_reports/test_toString.log

debug_test_toNumber : $(DEBUG_TEST_EXE)
	 @ echo "Run toNumber" > test_reports/test_toNumber.log
	 @ ./$(DEBUG_TEST_EXE) --run_test=toNumber --report_sin=stdout >> test_reports/test_toNumber.log

release_test_toNumber : $(RELEASE_TEST_EXE)
	 @ echo "Run toNumber" > test_reports/test_toNumber.log
	 @ ./$(RELEASE_TEST_EXE) --run_test=toNumber --report_sin=stdout >> test_reports/test_toNumber.log

debug_test_all: debug_test_bb_entry_exit_count debug_test_bb_iterators debug_test_bb_new_bb debug_test_bb_remove_bb debug_test_bb_at debug_test_ApplicationsSuite/applications_hangman debug_test_SamplesSuite/samples_arrays debug_test_SamplesSuite/samples_asm debug_test_SamplesSuite/samples_assembly debug_test_SamplesSuite/samples_bool debug_test_SamplesSuite/samples_compound debug_test_SamplesSuite/samples_concat debug_test_SamplesSuite/samples_const debug_test_SamplesSuite/samples_functions debug_test_SamplesSuite/samples_float debug_test_SamplesSuite/samples_little_float debug_test_SamplesSuite/samples_casts debug_test_SamplesSuite/samples_inc debug_test_SamplesSuite/samples_includes debug_test_SamplesSuite/samples_optimize debug_test_SamplesSuite/samples_problem debug_test_SamplesSuite/samples_identifiers debug_test_SamplesSuite/samples_registers debug_test_SamplesSuite/samples_structures debug_test_SpecificSuite/addressof debug_test_SpecificSuite/array_foreach_local debug_test_SpecificSuite/array_foreach_global debug_test_SpecificSuite/array_foreach_param_local debug_test_SpecificSuite/array_foreach_param_global debug_test_SpecificSuite/array_foreach_param_param debug_test_SpecificSuite/arrays_in_struct debug_test_SpecificSuite/char_type debug_test_SpecificSuite/char_at debug_test_SpecificSuite/ctor_dtor_heap debug_test_SpecificSuite/ctor_dtor_stack debug_test_SpecificSuite/copy_constructors debug_test_SpecificSuite/casts debug_test_SpecificSuite/compound debug_test_SpecificSuite/delete_any debug_test_SpecificSuite/if_ debug_test_SpecificSuite/includes debug_test_SpecificSuite/int_arrays debug_test_SpecificSuite/string_arrays debug_test_SpecificSuite/string_foreach debug_test_SpecificSuite/string_pointers debug_test_SpecificSuite/int_pointers debug_test_SpecificSuite/bool_pointers debug_test_SpecificSuite/cmove debug_test_SpecificSuite/dynamic debug_test_SpecificSuite/dynamic_arrays_in_struct debug_test_SpecificSuite/dynamic_struct debug_test_SpecificSuite/float_pointers debug_test_SpecificSuite/struct_pointers debug_test_SpecificSuite/member_pointers debug_test_SpecificSuite/member_function_calls debug_test_SpecificSuite/member_functions debug_test_SpecificSuite/member_functions_param_stack debug_test_SpecificSuite/memory debug_test_SpecificSuite/pass_member_by_value debug_test_SpecificSuite/ternary debug_test_SpecificSuite/while_ debug_test_SpecificSuite/do_while_ debug_test_SpecificSuite/defaults debug_test_SpecificSuite/float_1 debug_test_SpecificSuite/float_2 debug_test_SpecificSuite/for_ debug_test_SpecificSuite/foreach_ debug_test_SpecificSuite/globals_ debug_test_SpecificSuite/inc debug_test_SpecificSuite/void_ debug_test_SpecificSuite/return_string debug_test_SpecificSuite/return_by_value debug_test_SpecificSuite/return_int debug_test_SpecificSuite/return_pointers debug_test_SpecificSuite/pointer_arrays debug_test_SpecificSuite/recursive_functions debug_test_SpecificSuite/single_inheritance debug_test_SpecificSuite/math debug_test_SpecificSuite/builtin debug_test_SpecificSuite/assign_value debug_test_SpecificSuite/println debug_test_SpecificSuite/prints debug_test_SpecificSuite/structures debug_test_SpecificSuite/struct_member_pointers debug_test_SpecificSuite/struct_array debug_test_SpecificSuite/switch_ debug_test_SpecificSuite/switch_string debug_test_SpecificSuite/nested debug_test_SpecificSuite/args debug_test_TemplateSuite/class_templates debug_test_TemplateSuite/function_templates debug_test_TemplateSuite/member_function_templates debug_test_CompilationErrorsSuite/params_assign debug_test_CompilationErrorsSuite/wrong_print debug_test_CompilationErrorsSuite/invalid_inheritance debug_test_StandardLibSuite/std_lib_arrays_sum debug_test_StandardLibSuite/std_lib_math_min debug_test_StandardLibSuite/std_lib_math_max debug_test_StandardLibSuite/std_lib_math_factorial debug_test_StandardLibSuite/std_lib_math_pow debug_test_StandardLibSuite/std_lib_str_equals debug_test_StandardLibSuite/std_lib_string debug_test_StandardLibSuite/std_lib_string_concat debug_test_StandardLibSuite/std_lib_string_concat_int debug_test_StandardLibSuite/std_linked_list debug_test_StandardLibSuite/std_vector debug_test_BugFixesSuite/while_bug debug_test_OptimizationSuite/parameter_propagation debug_test_OptimizationSuite/global_cp debug_test_OptimizationSuite/global_offset_cp debug_test_OptimizationSuite/remove_empty_functions debug_test_OptimizationSuite/remove_empty_loops debug_test_OptimizationSuite/invariant_code_motion debug_test_OptimizationSuite/complete_loop_peeling debug_test_OptimizationSuite/complete_loop_peeling_2 debug_test_OptimizationSuite/loop_unrolling debug_test_OptimizationSuite/loop_unswitching debug_test_OptimizationSuite/global_cse debug_test_OptimizationSuite/local_cse debug_test_OptimizationSuite/cmov_opt debug_test_toString debug_test_toNumber 
	 @ bash ./tools/test_report.sh

release_test_all: release_test_bb_entry_exit_count release_test_bb_iterators release_test_bb_new_bb release_test_bb_remove_bb release_test_bb_at release_test_ApplicationsSuite/applications_hangman release_test_SamplesSuite/samples_arrays release_test_SamplesSuite/samples_asm release_test_SamplesSuite/samples_assembly release_test_SamplesSuite/samples_bool release_test_SamplesSuite/samples_compound release_test_SamplesSuite/samples_concat release_test_SamplesSuite/samples_const release_test_SamplesSuite/samples_functions release_test_SamplesSuite/samples_float release_test_SamplesSuite/samples_little_float release_test_SamplesSuite/samples_casts release_test_SamplesSuite/samples_inc release_test_SamplesSuite/samples_includes release_test_SamplesSuite/samples_optimize release_test_SamplesSuite/samples_problem release_test_SamplesSuite/samples_identifiers release_test_SamplesSuite/samples_registers release_test_SamplesSuite/samples_structures release_test_SpecificSuite/addressof release_test_SpecificSuite/array_foreach_local release_test_SpecificSuite/array_foreach_global release_test_SpecificSuite/array_foreach_param_local release_test_SpecificSuite/array_foreach_param_global release_test_SpecificSuite/array_foreach_param_param release_test_SpecificSuite/arrays_in_struct release_test_SpecificSuite/char_type release_test_SpecificSuite/char_at release_test_SpecificSuite/ctor_dtor_heap release_test_SpecificSuite/ctor_dtor_stack release_test_SpecificSuite/copy_constructors release_test_SpecificSuite/casts release_test_SpecificSuite/compound release_test_SpecificSuite/delete_any release_test_SpecificSuite/if_ release_test_SpecificSuite/includes release_test_SpecificSuite/int_arrays release_test_SpecificSuite/string_arrays release_test_SpecificSuite/string_foreach release_test_SpecificSuite/string_pointers release_test_SpecificSuite/int_pointers release_test_SpecificSuite/bool_pointers release_test_SpecificSuite/cmove release_test_SpecificSuite/dynamic release_test_SpecificSuite/dynamic_arrays_in_struct release_test_SpecificSuite/dynamic_struct release_test_SpecificSuite/float_pointers release_test_SpecificSuite/struct_pointers release_test_SpecificSuite/member_pointers release_test_SpecificSuite/member_function_calls release_test_SpecificSuite/member_functions release_test_SpecificSuite/member_functions_param_stack release_test_SpecificSuite/memory release_test_SpecificSuite/pass_member_by_value release_test_SpecificSuite/ternary release_test_SpecificSuite/while_ release_test_SpecificSuite/do_while_ release_test_SpecificSuite/defaults release_test_SpecificSuite/float_1 release_test_SpecificSuite/float_2 release_test_SpecificSuite/for_ release_test_SpecificSuite/foreach_ release_test_SpecificSuite/globals_ release_test_SpecificSuite/inc release_test_SpecificSuite/void_ release_test_SpecificSuite/return_string release_test_SpecificSuite/return_by_value release_test_SpecificSuite/return_int release_test_SpecificSuite/return_pointers release_test_SpecificSuite/pointer_arrays release_test_SpecificSuite/recursive_functions release_test_SpecificSuite/single_inheritance release_test_SpecificSuite/math release_test_SpecificSuite/builtin release_test_SpecificSuite/assign_value release_test_SpecificSuite/println release_test_SpecificSuite/prints release_test_SpecificSuite/structures release_test_SpecificSuite/struct_member_pointers release_test_SpecificSuite/struct_array release_test_SpecificSuite/switch_ release_test_SpecificSuite/switch_string release_test_SpecificSuite/nested release_test_SpecificSuite/args release_test_TemplateSuite/class_templates release_test_TemplateSuite/function_templates release_test_TemplateSuite/member_function_templates release_test_CompilationErrorsSuite/params_assign release_test_CompilationErrorsSuite/wrong_print release_test_CompilationErrorsSuite/invalid_inheritance release_test_StandardLibSuite/std_lib_arrays_sum release_test_StandardLibSuite/std_lib_math_min release_test_StandardLibSuite/std_lib_math_max release_test_StandardLibSuite/std_lib_math_factorial release_test_StandardLibSuite/std_lib_math_pow release_test_StandardLibSuite/std_lib_str_equals release_test_StandardLibSuite/std_lib_string release_test_StandardLibSuite/std_lib_string_concat release_test_StandardLibSuite/std_lib_string_concat_int release_test_StandardLibSuite/std_linked_list release_test_StandardLibSuite/std_vector release_test_BugFixesSuite/while_bug release_test_OptimizationSuite/parameter_propagation release_test_OptimizationSuite/global_cp release_test_OptimizationSuite/global_offset_cp release_test_OptimizationSuite/remove_empty_functions release_test_OptimizationSuite/remove_empty_loops release_test_OptimizationSuite/invariant_code_motion release_test_OptimizationSuite/complete_loop_peeling release_test_OptimizationSuite/complete_loop_peeling_2 release_test_OptimizationSuite/loop_unrolling release_test_OptimizationSuite/loop_unswitching release_test_OptimizationSuite/global_cse release_test_OptimizationSuite/local_cse release_test_OptimizationSuite/cmov_opt release_test_toString release_test_toNumber 
	 @ bash ./tools/test_report.sh

.PHONY: release_test_all debug_test_alldebug_test_bb_entry_exit_count release_test_bb_entry_exit_count debug_test_bb_iterators release_test_bb_iterators debug_test_bb_new_bb release_test_bb_new_bb debug_test_bb_remove_bb release_test_bb_remove_bb debug_test_bb_at release_test_bb_at debug_test_ApplicationsSuite/applications_hangman release_test_ApplicationsSuite/applications_hangman debug_test_SamplesSuite/samples_arrays release_test_SamplesSuite/samples_arrays debug_test_SamplesSuite/samples_asm release_test_SamplesSuite/samples_asm debug_test_SamplesSuite/samples_assembly release_test_SamplesSuite/samples_assembly debug_test_SamplesSuite/samples_bool release_test_SamplesSuite/samples_bool debug_test_SamplesSuite/samples_compound release_test_SamplesSuite/samples_compound debug_test_SamplesSuite/samples_concat release_test_SamplesSuite/samples_concat debug_test_SamplesSuite/samples_const release_test_SamplesSuite/samples_const debug_test_SamplesSuite/samples_functions release_test_SamplesSuite/samples_functions debug_test_SamplesSuite/samples_float release_test_SamplesSuite/samples_float debug_test_SamplesSuite/samples_little_float release_test_SamplesSuite/samples_little_float debug_test_SamplesSuite/samples_casts release_test_SamplesSuite/samples_casts debug_test_SamplesSuite/samples_inc release_test_SamplesSuite/samples_inc debug_test_SamplesSuite/samples_includes release_test_SamplesSuite/samples_includes debug_test_SamplesSuite/samples_optimize release_test_SamplesSuite/samples_optimize debug_test_SamplesSuite/samples_problem release_test_SamplesSuite/samples_problem debug_test_SamplesSuite/samples_identifiers release_test_SamplesSuite/samples_identifiers debug_test_SamplesSuite/samples_registers release_test_SamplesSuite/samples_registers debug_test_SamplesSuite/samples_structures release_test_SamplesSuite/samples_structures debug_test_SpecificSuite/addressof release_test_SpecificSuite/addressof debug_test_SpecificSuite/array_foreach_local release_test_SpecificSuite/array_foreach_local debug_test_SpecificSuite/array_foreach_global release_test_SpecificSuite/array_foreach_global debug_test_SpecificSuite/array_foreach_param_local release_test_SpecificSuite/array_foreach_param_local debug_test_SpecificSuite/array_foreach_param_global release_test_SpecificSuite/array_foreach_param_global debug_test_SpecificSuite/array_foreach_param_param release_test_SpecificSuite/array_foreach_param_param debug_test_SpecificSuite/arrays_in_struct release_test_SpecificSuite/arrays_in_struct debug_test_SpecificSuite/char_type release_test_SpecificSuite/char_type debug_test_SpecificSuite/char_at release_test_SpecificSuite/char_at debug_test_SpecificSuite/ctor_dtor_heap release_test_SpecificSuite/ctor_dtor_heap debug_test_SpecificSuite/ctor_dtor_stack release_test_SpecificSuite/ctor_dtor_stack debug_test_SpecificSuite/copy_constructors release_test_SpecificSuite/copy_constructors debug_test_SpecificSuite/casts release_test_SpecificSuite/casts debug_test_SpecificSuite/compound release_test_SpecificSuite/compound debug_test_SpecificSuite/delete_any release_test_SpecificSuite/delete_any debug_test_SpecificSuite/if_ release_test_SpecificSuite/if_ debug_test_SpecificSuite/includes release_test_SpecificSuite/includes debug_test_SpecificSuite/int_arrays release_test_SpecificSuite/int_arrays debug_test_SpecificSuite/string_arrays release_test_SpecificSuite/string_arrays debug_test_SpecificSuite/string_foreach release_test_SpecificSuite/string_foreach debug_test_SpecificSuite/string_pointers release_test_SpecificSuite/string_pointers debug_test_SpecificSuite/int_pointers release_test_SpecificSuite/int_pointers debug_test_SpecificSuite/bool_pointers release_test_SpecificSuite/bool_pointers debug_test_SpecificSuite/cmove release_test_SpecificSuite/cmove debug_test_SpecificSuite/dynamic release_test_SpecificSuite/dynamic debug_test_SpecificSuite/dynamic_arrays_in_struct release_test_SpecificSuite/dynamic_arrays_in_struct debug_test_SpecificSuite/dynamic_struct release_test_SpecificSuite/dynamic_struct debug_test_SpecificSuite/float_pointers release_test_SpecificSuite/float_pointers debug_test_SpecificSuite/struct_pointers release_test_SpecificSuite/struct_pointers debug_test_SpecificSuite/member_pointers release_test_SpecificSuite/member_pointers debug_test_SpecificSuite/member_function_calls release_test_SpecificSuite/member_function_calls debug_test_SpecificSuite/member_functions release_test_SpecificSuite/member_functions debug_test_SpecificSuite/member_functions_param_stack release_test_SpecificSuite/member_functions_param_stack debug_test_SpecificSuite/memory release_test_SpecificSuite/memory debug_test_SpecificSuite/pass_member_by_value release_test_SpecificSuite/pass_member_by_value debug_test_SpecificSuite/ternary release_test_SpecificSuite/ternary debug_test_SpecificSuite/while_ release_test_SpecificSuite/while_ debug_test_SpecificSuite/do_while_ release_test_SpecificSuite/do_while_ debug_test_SpecificSuite/defaults release_test_SpecificSuite/defaults debug_test_SpecificSuite/float_1 release_test_SpecificSuite/float_1 debug_test_SpecificSuite/float_2 release_test_SpecificSuite/float_2 debug_test_SpecificSuite/for_ release_test_SpecificSuite/for_ debug_test_SpecificSuite/foreach_ release_test_SpecificSuite/foreach_ debug_test_SpecificSuite/globals_ release_test_SpecificSuite/globals_ debug_test_SpecificSuite/inc release_test_SpecificSuite/inc debug_test_SpecificSuite/void_ release_test_SpecificSuite/void_ debug_test_SpecificSuite/return_string release_test_SpecificSuite/return_string debug_test_SpecificSuite/return_by_value release_test_SpecificSuite/return_by_value debug_test_SpecificSuite/return_int release_test_SpecificSuite/return_int debug_test_SpecificSuite/return_pointers release_test_SpecificSuite/return_pointers debug_test_SpecificSuite/pointer_arrays release_test_SpecificSuite/pointer_arrays debug_test_SpecificSuite/recursive_functions release_test_SpecificSuite/recursive_functions debug_test_SpecificSuite/single_inheritance release_test_SpecificSuite/single_inheritance debug_test_SpecificSuite/math release_test_SpecificSuite/math debug_test_SpecificSuite/builtin release_test_SpecificSuite/builtin debug_test_SpecificSuite/assign_value release_test_SpecificSuite/assign_value debug_test_SpecificSuite/println release_test_SpecificSuite/println debug_test_SpecificSuite/prints release_test_SpecificSuite/prints debug_test_SpecificSuite/structures release_test_SpecificSuite/structures debug_test_SpecificSuite/struct_member_pointers release_test_SpecificSuite/struct_member_pointers debug_test_SpecificSuite/struct_array release_test_SpecificSuite/struct_array debug_test_SpecificSuite/switch_ release_test_SpecificSuite/switch_ debug_test_SpecificSuite/switch_string release_test_SpecificSuite/switch_string debug_test_SpecificSuite/nested release_test_SpecificSuite/nested debug_test_SpecificSuite/args release_test_SpecificSuite/args debug_test_TemplateSuite/class_templates release_test_TemplateSuite/class_templates debug_test_TemplateSuite/function_templates release_test_TemplateSuite/function_templates debug_test_TemplateSuite/member_function_templates release_test_TemplateSuite/member_function_templates debug_test_CompilationErrorsSuite/params_assign release_test_CompilationErrorsSuite/params_assign debug_test_CompilationErrorsSuite/wrong_print release_test_CompilationErrorsSuite/wrong_print debug_test_CompilationErrorsSuite/invalid_inheritance release_test_CompilationErrorsSuite/invalid_inheritance debug_test_StandardLibSuite/std_lib_arrays_sum release_test_StandardLibSuite/std_lib_arrays_sum debug_test_StandardLibSuite/std_lib_math_min release_test_StandardLibSuite/std_lib_math_min debug_test_StandardLibSuite/std_lib_math_max release_test_StandardLibSuite/std_lib_math_max debug_test_StandardLibSuite/std_lib_math_factorial release_test_StandardLibSuite/std_lib_math_factorial debug_test_StandardLibSuite/std_lib_math_pow release_test_StandardLibSuite/std_lib_math_pow debug_test_StandardLibSuite/std_lib_str_equals release_test_StandardLibSuite/std_lib_str_equals debug_test_StandardLibSuite/std_lib_string release_test_StandardLibSuite/std_lib_string debug_test_StandardLibSuite/std_lib_string_concat release_test_StandardLibSuite/std_lib_string_concat debug_test_StandardLibSuite/std_lib_string_concat_int release_test_StandardLibSuite/std_lib_string_concat_int debug_test_StandardLibSuite/std_linked_list release_test_StandardLibSuite/std_linked_list debug_test_StandardLibSuite/std_vector release_test_StandardLibSuite/std_vector debug_test_BugFixesSuite/while_bug release_test_BugFixesSuite/while_bug debug_test_OptimizationSuite/parameter_propagation release_test_OptimizationSuite/parameter_propagation debug_test_OptimizationSuite/global_cp release_test_OptimizationSuite/global_cp debug_test_OptimizationSuite/global_offset_cp release_test_OptimizationSuite/global_offset_cp debug_test_OptimizationSuite/remove_empty_functions release_test_OptimizationSuite/remove_empty_functions debug_test_OptimizationSuite/remove_empty_loops release_test_OptimizationSuite/remove_empty_loops debug_test_OptimizationSuite/invariant_code_motion release_test_OptimizationSuite/invariant_code_motion debug_test_OptimizationSuite/complete_loop_peeling release_test_OptimizationSuite/complete_loop_peeling debug_test_OptimizationSuite/complete_loop_peeling_2 release_test_OptimizationSuite/complete_loop_peeling_2 debug_test_OptimizationSuite/loop_unrolling release_test_OptimizationSuite/loop_unrolling debug_test_OptimizationSuite/loop_unswitching release_test_OptimizationSuite/loop_unswitching debug_test_OptimizationSuite/global_cse release_test_OptimizationSuite/global_cse debug_test_OptimizationSuite/local_cse release_test_OptimizationSuite/local_cse debug_test_OptimizationSuite/cmov_opt release_test_OptimizationSuite/cmov_opt debug_test_toString release_test_toString debug_test_toNumber release_test_toNumber 
