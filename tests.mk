test_bb_entry_exit_count : $(TEST_EXE)
	 @ echo "Run bb_entry_exit_count" > test_reports/test_bb_entry_exit_count.log
	 @ ./$(TEST_EXE) --run_test=bb_entry_exit_count --report_sin=stdout >> test_reports/test_bb_entry_exit_count.log

test_bb_iterators : $(TEST_EXE)
	 @ echo "Run bb_iterators" > test_reports/test_bb_iterators.log
	 @ ./$(TEST_EXE) --run_test=bb_iterators --report_sin=stdout >> test_reports/test_bb_iterators.log

test_bb_new_bb : $(TEST_EXE)
	 @ echo "Run bb_new_bb" > test_reports/test_bb_new_bb.log
	 @ ./$(TEST_EXE) --run_test=bb_new_bb --report_sin=stdout >> test_reports/test_bb_new_bb.log

test_bb_remove_bb : $(TEST_EXE)
	 @ echo "Run bb_remove_bb" > test_reports/test_bb_remove_bb.log
	 @ ./$(TEST_EXE) --run_test=bb_remove_bb --report_sin=stdout >> test_reports/test_bb_remove_bb.log

test_bb_at : $(TEST_EXE)
	 @ echo "Run bb_at" > test_reports/test_bb_at.log
	 @ ./$(TEST_EXE) --run_test=bb_at --report_sin=stdout >> test_reports/test_bb_at.log

test_ApplicationsSuite/applications_hangman : $(TEST_EXE)
	 @ echo "Run ApplicationsSuite/applications_hangman" > test_reports/test_ApplicationsSuite-applications_hangman.log
	 @ ./$(TEST_EXE) --run_test=ApplicationsSuite/applications_hangman --report_sin=stdout >> test_reports/test_ApplicationsSuite-applications_hangman.log

test_SamplesSuite/samples_arrays : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_arrays" > test_reports/test_SamplesSuite-samples_arrays.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_arrays --report_sin=stdout >> test_reports/test_SamplesSuite-samples_arrays.log

test_SamplesSuite/samples_asm : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_asm" > test_reports/test_SamplesSuite-samples_asm.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_asm --report_sin=stdout >> test_reports/test_SamplesSuite-samples_asm.log

test_SamplesSuite/samples_assembly : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_assembly" > test_reports/test_SamplesSuite-samples_assembly.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_assembly --report_sin=stdout >> test_reports/test_SamplesSuite-samples_assembly.log

test_SamplesSuite/samples_bool : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_bool" > test_reports/test_SamplesSuite-samples_bool.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_bool --report_sin=stdout >> test_reports/test_SamplesSuite-samples_bool.log

test_SamplesSuite/samples_compound : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_compound" > test_reports/test_SamplesSuite-samples_compound.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_compound --report_sin=stdout >> test_reports/test_SamplesSuite-samples_compound.log

test_SamplesSuite/samples_concat : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_concat" > test_reports/test_SamplesSuite-samples_concat.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_concat --report_sin=stdout >> test_reports/test_SamplesSuite-samples_concat.log

test_SamplesSuite/samples_const : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_const" > test_reports/test_SamplesSuite-samples_const.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_const --report_sin=stdout >> test_reports/test_SamplesSuite-samples_const.log

test_SamplesSuite/samples_functions : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_functions" > test_reports/test_SamplesSuite-samples_functions.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_functions --report_sin=stdout >> test_reports/test_SamplesSuite-samples_functions.log

test_SamplesSuite/samples_float : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_float" > test_reports/test_SamplesSuite-samples_float.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_float.log

test_SamplesSuite/samples_little_float : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_little_float" > test_reports/test_SamplesSuite-samples_little_float.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_little_float --report_sin=stdout >> test_reports/test_SamplesSuite-samples_little_float.log

test_SamplesSuite/samples_casts : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_casts" > test_reports/test_SamplesSuite-samples_casts.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_casts --report_sin=stdout >> test_reports/test_SamplesSuite-samples_casts.log

test_SamplesSuite/samples_inc : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_inc" > test_reports/test_SamplesSuite-samples_inc.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_inc --report_sin=stdout >> test_reports/test_SamplesSuite-samples_inc.log

test_SamplesSuite/samples_includes : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_includes" > test_reports/test_SamplesSuite-samples_includes.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_includes --report_sin=stdout >> test_reports/test_SamplesSuite-samples_includes.log

test_SamplesSuite/samples_optimize : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_optimize" > test_reports/test_SamplesSuite-samples_optimize.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_optimize --report_sin=stdout >> test_reports/test_SamplesSuite-samples_optimize.log

test_SamplesSuite/samples_problem : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_problem" > test_reports/test_SamplesSuite-samples_problem.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_problem --report_sin=stdout >> test_reports/test_SamplesSuite-samples_problem.log

test_SamplesSuite/samples_identifiers : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_identifiers" > test_reports/test_SamplesSuite-samples_identifiers.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_identifiers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_identifiers.log

test_SamplesSuite/samples_registers : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_registers" > test_reports/test_SamplesSuite-samples_registers.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_registers --report_sin=stdout >> test_reports/test_SamplesSuite-samples_registers.log

test_SamplesSuite/samples_structures : $(TEST_EXE)
	 @ echo "Run SamplesSuite/samples_structures" > test_reports/test_SamplesSuite-samples_structures.log
	 @ ./$(TEST_EXE) --run_test=SamplesSuite/samples_structures --report_sin=stdout >> test_reports/test_SamplesSuite-samples_structures.log

test_SpecificSuite/addressof : $(TEST_EXE)
	 @ echo "Run SpecificSuite/addressof" > test_reports/test_SpecificSuite-addressof.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/addressof --report_sin=stdout >> test_reports/test_SpecificSuite-addressof.log

test_SpecificSuite/array_foreach_local : $(TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_local" > test_reports/test_SpecificSuite-array_foreach_local.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/array_foreach_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_local.log

test_SpecificSuite/array_foreach_global : $(TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_global" > test_reports/test_SpecificSuite-array_foreach_global.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/array_foreach_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_global.log

test_SpecificSuite/array_foreach_param_local : $(TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_local" > test_reports/test_SpecificSuite-array_foreach_param_local.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/array_foreach_param_local --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_local.log

test_SpecificSuite/array_foreach_param_global : $(TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_global" > test_reports/test_SpecificSuite-array_foreach_param_global.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/array_foreach_param_global --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_global.log

test_SpecificSuite/array_foreach_param_param : $(TEST_EXE)
	 @ echo "Run SpecificSuite/array_foreach_param_param" > test_reports/test_SpecificSuite-array_foreach_param_param.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/array_foreach_param_param --report_sin=stdout >> test_reports/test_SpecificSuite-array_foreach_param_param.log

test_SpecificSuite/arrays_in_struct : $(TEST_EXE)
	 @ echo "Run SpecificSuite/arrays_in_struct" > test_reports/test_SpecificSuite-arrays_in_struct.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-arrays_in_struct.log

test_SpecificSuite/char_type : $(TEST_EXE)
	 @ echo "Run SpecificSuite/char_type" > test_reports/test_SpecificSuite-char_type.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/char_type --report_sin=stdout >> test_reports/test_SpecificSuite-char_type.log

test_SpecificSuite/char_at : $(TEST_EXE)
	 @ echo "Run SpecificSuite/char_at" > test_reports/test_SpecificSuite-char_at.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/char_at --report_sin=stdout >> test_reports/test_SpecificSuite-char_at.log

test_SpecificSuite/ctor_dtor_heap : $(TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_heap" > test_reports/test_SpecificSuite-ctor_dtor_heap.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/ctor_dtor_heap --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_heap.log

test_SpecificSuite/ctor_dtor_stack : $(TEST_EXE)
	 @ echo "Run SpecificSuite/ctor_dtor_stack" > test_reports/test_SpecificSuite-ctor_dtor_stack.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/ctor_dtor_stack --report_sin=stdout >> test_reports/test_SpecificSuite-ctor_dtor_stack.log

test_SpecificSuite/copy_constructors : $(TEST_EXE)
	 @ echo "Run SpecificSuite/copy_constructors" > test_reports/test_SpecificSuite-copy_constructors.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/copy_constructors --report_sin=stdout >> test_reports/test_SpecificSuite-copy_constructors.log

test_SpecificSuite/casts : $(TEST_EXE)
	 @ echo "Run SpecificSuite/casts" > test_reports/test_SpecificSuite-casts.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/casts --report_sin=stdout >> test_reports/test_SpecificSuite-casts.log

test_SpecificSuite/compound : $(TEST_EXE)
	 @ echo "Run SpecificSuite/compound" > test_reports/test_SpecificSuite-compound.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/compound --report_sin=stdout >> test_reports/test_SpecificSuite-compound.log

test_SpecificSuite/delete_any : $(TEST_EXE)
	 @ echo "Run SpecificSuite/delete_any" > test_reports/test_SpecificSuite-delete_any.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/delete_any --report_sin=stdout >> test_reports/test_SpecificSuite-delete_any.log

test_SpecificSuite/if_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/if_" > test_reports/test_SpecificSuite-if_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/if_ --report_sin=stdout >> test_reports/test_SpecificSuite-if_.log

test_SpecificSuite/includes : $(TEST_EXE)
	 @ echo "Run SpecificSuite/includes" > test_reports/test_SpecificSuite-includes.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/includes --report_sin=stdout >> test_reports/test_SpecificSuite-includes.log

test_SpecificSuite/int_arrays : $(TEST_EXE)
	 @ echo "Run SpecificSuite/int_arrays" > test_reports/test_SpecificSuite-int_arrays.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/int_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-int_arrays.log

test_SpecificSuite/string_arrays : $(TEST_EXE)
	 @ echo "Run SpecificSuite/string_arrays" > test_reports/test_SpecificSuite-string_arrays.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/string_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-string_arrays.log

test_SpecificSuite/string_foreach : $(TEST_EXE)
	 @ echo "Run SpecificSuite/string_foreach" > test_reports/test_SpecificSuite-string_foreach.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/string_foreach --report_sin=stdout >> test_reports/test_SpecificSuite-string_foreach.log

test_SpecificSuite/string_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/string_pointers" > test_reports/test_SpecificSuite-string_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/string_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-string_pointers.log

test_SpecificSuite/int_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/int_pointers" > test_reports/test_SpecificSuite-int_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/int_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-int_pointers.log

test_SpecificSuite/bool_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/bool_pointers" > test_reports/test_SpecificSuite-bool_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/bool_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-bool_pointers.log

test_SpecificSuite/cmove : $(TEST_EXE)
	 @ echo "Run SpecificSuite/cmove" > test_reports/test_SpecificSuite-cmove.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/cmove --report_sin=stdout >> test_reports/test_SpecificSuite-cmove.log

test_SpecificSuite/dynamic : $(TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic" > test_reports/test_SpecificSuite-dynamic.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/dynamic --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic.log

test_SpecificSuite/dynamic_arrays_in_struct : $(TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_arrays_in_struct" > test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/dynamic_arrays_in_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_arrays_in_struct.log

test_SpecificSuite/dynamic_struct : $(TEST_EXE)
	 @ echo "Run SpecificSuite/dynamic_struct" > test_reports/test_SpecificSuite-dynamic_struct.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/dynamic_struct --report_sin=stdout >> test_reports/test_SpecificSuite-dynamic_struct.log

test_SpecificSuite/float_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/float_pointers" > test_reports/test_SpecificSuite-float_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/float_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-float_pointers.log

test_SpecificSuite/struct_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/struct_pointers" > test_reports/test_SpecificSuite-struct_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/struct_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_pointers.log

test_SpecificSuite/member_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/member_pointers" > test_reports/test_SpecificSuite-member_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-member_pointers.log

test_SpecificSuite/member_function_calls : $(TEST_EXE)
	 @ echo "Run SpecificSuite/member_function_calls" > test_reports/test_SpecificSuite-member_function_calls.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/member_function_calls --report_sin=stdout >> test_reports/test_SpecificSuite-member_function_calls.log

test_SpecificSuite/member_functions : $(TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions" > test_reports/test_SpecificSuite-member_functions.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/member_functions --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions.log

test_SpecificSuite/member_functions_param_stack : $(TEST_EXE)
	 @ echo "Run SpecificSuite/member_functions_param_stack" > test_reports/test_SpecificSuite-member_functions_param_stack.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/member_functions_param_stack --report_sin=stdout >> test_reports/test_SpecificSuite-member_functions_param_stack.log

test_SpecificSuite/memory : $(TEST_EXE)
	 @ echo "Run SpecificSuite/memory" > test_reports/test_SpecificSuite-memory.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/memory --report_sin=stdout >> test_reports/test_SpecificSuite-memory.log

test_SpecificSuite/pass_member_by_value : $(TEST_EXE)
	 @ echo "Run SpecificSuite/pass_member_by_value" > test_reports/test_SpecificSuite-pass_member_by_value.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/pass_member_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-pass_member_by_value.log

test_SpecificSuite/ternary : $(TEST_EXE)
	 @ echo "Run SpecificSuite/ternary" > test_reports/test_SpecificSuite-ternary.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/ternary --report_sin=stdout >> test_reports/test_SpecificSuite-ternary.log

test_SpecificSuite/while_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/while_" > test_reports/test_SpecificSuite-while_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/while_ --report_sin=stdout >> test_reports/test_SpecificSuite-while_.log

test_SpecificSuite/do_while_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/do_while_" > test_reports/test_SpecificSuite-do_while_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/do_while_ --report_sin=stdout >> test_reports/test_SpecificSuite-do_while_.log

test_SpecificSuite/defaults : $(TEST_EXE)
	 @ echo "Run SpecificSuite/defaults" > test_reports/test_SpecificSuite-defaults.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/defaults --report_sin=stdout >> test_reports/test_SpecificSuite-defaults.log

test_SpecificSuite/float_1 : $(TEST_EXE)
	 @ echo "Run SpecificSuite/float_1" > test_reports/test_SpecificSuite-float_1.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/float_1 --report_sin=stdout >> test_reports/test_SpecificSuite-float_1.log

test_SpecificSuite/float_2 : $(TEST_EXE)
	 @ echo "Run SpecificSuite/float_2" > test_reports/test_SpecificSuite-float_2.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/float_2 --report_sin=stdout >> test_reports/test_SpecificSuite-float_2.log

test_SpecificSuite/for_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/for_" > test_reports/test_SpecificSuite-for_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/for_ --report_sin=stdout >> test_reports/test_SpecificSuite-for_.log

test_SpecificSuite/foreach_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/foreach_" > test_reports/test_SpecificSuite-foreach_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/foreach_ --report_sin=stdout >> test_reports/test_SpecificSuite-foreach_.log

test_SpecificSuite/globals_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/globals_" > test_reports/test_SpecificSuite-globals_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/globals_ --report_sin=stdout >> test_reports/test_SpecificSuite-globals_.log

test_SpecificSuite/inc : $(TEST_EXE)
	 @ echo "Run SpecificSuite/inc" > test_reports/test_SpecificSuite-inc.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/inc --report_sin=stdout >> test_reports/test_SpecificSuite-inc.log

test_SpecificSuite/void_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/void_" > test_reports/test_SpecificSuite-void_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/void_ --report_sin=stdout >> test_reports/test_SpecificSuite-void_.log

test_SpecificSuite/return_string : $(TEST_EXE)
	 @ echo "Run SpecificSuite/return_string" > test_reports/test_SpecificSuite-return_string.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/return_string --report_sin=stdout >> test_reports/test_SpecificSuite-return_string.log

test_SpecificSuite/return_by_value : $(TEST_EXE)
	 @ echo "Run SpecificSuite/return_by_value" > test_reports/test_SpecificSuite-return_by_value.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/return_by_value --report_sin=stdout >> test_reports/test_SpecificSuite-return_by_value.log

test_SpecificSuite/return_int : $(TEST_EXE)
	 @ echo "Run SpecificSuite/return_int" > test_reports/test_SpecificSuite-return_int.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/return_int --report_sin=stdout >> test_reports/test_SpecificSuite-return_int.log

test_SpecificSuite/return_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/return_pointers" > test_reports/test_SpecificSuite-return_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/return_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-return_pointers.log

test_SpecificSuite/pointer_arrays : $(TEST_EXE)
	 @ echo "Run SpecificSuite/pointer_arrays" > test_reports/test_SpecificSuite-pointer_arrays.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/pointer_arrays --report_sin=stdout >> test_reports/test_SpecificSuite-pointer_arrays.log

test_SpecificSuite/recursive_functions : $(TEST_EXE)
	 @ echo "Run SpecificSuite/recursive_functions" > test_reports/test_SpecificSuite-recursive_functions.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/recursive_functions --report_sin=stdout >> test_reports/test_SpecificSuite-recursive_functions.log

test_SpecificSuite/single_inheritance : $(TEST_EXE)
	 @ echo "Run SpecificSuite/single_inheritance" > test_reports/test_SpecificSuite-single_inheritance.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/single_inheritance --report_sin=stdout >> test_reports/test_SpecificSuite-single_inheritance.log

test_SpecificSuite/math : $(TEST_EXE)
	 @ echo "Run SpecificSuite/math" > test_reports/test_SpecificSuite-math.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/math --report_sin=stdout >> test_reports/test_SpecificSuite-math.log

test_SpecificSuite/builtin : $(TEST_EXE)
	 @ echo "Run SpecificSuite/builtin" > test_reports/test_SpecificSuite-builtin.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/builtin --report_sin=stdout >> test_reports/test_SpecificSuite-builtin.log

test_SpecificSuite/assign_value : $(TEST_EXE)
	 @ echo "Run SpecificSuite/assign_value" > test_reports/test_SpecificSuite-assign_value.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/assign_value --report_sin=stdout >> test_reports/test_SpecificSuite-assign_value.log

test_SpecificSuite/println : $(TEST_EXE)
	 @ echo "Run SpecificSuite/println" > test_reports/test_SpecificSuite-println.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/println --report_sin=stdout >> test_reports/test_SpecificSuite-println.log

test_SpecificSuite/prints : $(TEST_EXE)
	 @ echo "Run SpecificSuite/prints" > test_reports/test_SpecificSuite-prints.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/prints --report_sin=stdout >> test_reports/test_SpecificSuite-prints.log

test_SpecificSuite/structures : $(TEST_EXE)
	 @ echo "Run SpecificSuite/structures" > test_reports/test_SpecificSuite-structures.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/structures --report_sin=stdout >> test_reports/test_SpecificSuite-structures.log

test_SpecificSuite/struct_member_pointers : $(TEST_EXE)
	 @ echo "Run SpecificSuite/struct_member_pointers" > test_reports/test_SpecificSuite-struct_member_pointers.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/struct_member_pointers --report_sin=stdout >> test_reports/test_SpecificSuite-struct_member_pointers.log

test_SpecificSuite/struct_array : $(TEST_EXE)
	 @ echo "Run SpecificSuite/struct_array" > test_reports/test_SpecificSuite-struct_array.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/struct_array --report_sin=stdout >> test_reports/test_SpecificSuite-struct_array.log

test_SpecificSuite/switch_ : $(TEST_EXE)
	 @ echo "Run SpecificSuite/switch_" > test_reports/test_SpecificSuite-switch_.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/switch_ --report_sin=stdout >> test_reports/test_SpecificSuite-switch_.log

test_SpecificSuite/switch_string : $(TEST_EXE)
	 @ echo "Run SpecificSuite/switch_string" > test_reports/test_SpecificSuite-switch_string.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/switch_string --report_sin=stdout >> test_reports/test_SpecificSuite-switch_string.log

test_SpecificSuite/nested : $(TEST_EXE)
	 @ echo "Run SpecificSuite/nested" > test_reports/test_SpecificSuite-nested.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/nested --report_sin=stdout >> test_reports/test_SpecificSuite-nested.log

test_SpecificSuite/args : $(TEST_EXE)
	 @ echo "Run SpecificSuite/args" > test_reports/test_SpecificSuite-args.log
	 @ ./$(TEST_EXE) --run_test=SpecificSuite/args --report_sin=stdout >> test_reports/test_SpecificSuite-args.log

test_TemplateSuite/class_templates : $(TEST_EXE)
	 @ echo "Run TemplateSuite/class_templates" > test_reports/test_TemplateSuite-class_templates.log
	 @ ./$(TEST_EXE) --run_test=TemplateSuite/class_templates --report_sin=stdout >> test_reports/test_TemplateSuite-class_templates.log

test_TemplateSuite/function_templates : $(TEST_EXE)
	 @ echo "Run TemplateSuite/function_templates" > test_reports/test_TemplateSuite-function_templates.log
	 @ ./$(TEST_EXE) --run_test=TemplateSuite/function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-function_templates.log

test_TemplateSuite/member_function_templates : $(TEST_EXE)
	 @ echo "Run TemplateSuite/member_function_templates" > test_reports/test_TemplateSuite-member_function_templates.log
	 @ ./$(TEST_EXE) --run_test=TemplateSuite/member_function_templates --report_sin=stdout >> test_reports/test_TemplateSuite-member_function_templates.log

test_CompilationErrorsSuite/params_assign : $(TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/params_assign" > test_reports/test_CompilationErrorsSuite-params_assign.log
	 @ ./$(TEST_EXE) --run_test=CompilationErrorsSuite/params_assign --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-params_assign.log

test_CompilationErrorsSuite/wrong_print : $(TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/wrong_print" > test_reports/test_CompilationErrorsSuite-wrong_print.log
	 @ ./$(TEST_EXE) --run_test=CompilationErrorsSuite/wrong_print --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-wrong_print.log

test_CompilationErrorsSuite/invalid_inheritance : $(TEST_EXE)
	 @ echo "Run CompilationErrorsSuite/invalid_inheritance" > test_reports/test_CompilationErrorsSuite-invalid_inheritance.log
	 @ ./$(TEST_EXE) --run_test=CompilationErrorsSuite/invalid_inheritance --report_sin=stdout >> test_reports/test_CompilationErrorsSuite-invalid_inheritance.log

test_StandardLibSuite/std_lib_arrays_sum : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_arrays_sum" > test_reports/test_StandardLibSuite-std_lib_arrays_sum.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_arrays_sum --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_arrays_sum.log

test_StandardLibSuite/std_lib_math_min : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_min" > test_reports/test_StandardLibSuite-std_lib_math_min.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_math_min --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_min.log

test_StandardLibSuite/std_lib_math_max : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_max" > test_reports/test_StandardLibSuite-std_lib_math_max.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_math_max --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_max.log

test_StandardLibSuite/std_lib_math_factorial : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_factorial" > test_reports/test_StandardLibSuite-std_lib_math_factorial.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_math_factorial --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_factorial.log

test_StandardLibSuite/std_lib_math_pow : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_math_pow" > test_reports/test_StandardLibSuite-std_lib_math_pow.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_math_pow --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_math_pow.log

test_StandardLibSuite/std_lib_str_equals : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_str_equals" > test_reports/test_StandardLibSuite-std_lib_str_equals.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_str_equals --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_str_equals.log

test_StandardLibSuite/std_lib_string : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string" > test_reports/test_StandardLibSuite-std_lib_string.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_string --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string.log

test_StandardLibSuite/std_lib_string_concat : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat" > test_reports/test_StandardLibSuite-std_lib_string_concat.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat.log

test_StandardLibSuite/std_lib_string_concat_int : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_lib_string_concat_int" > test_reports/test_StandardLibSuite-std_lib_string_concat_int.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_lib_string_concat_int --report_sin=stdout >> test_reports/test_StandardLibSuite-std_lib_string_concat_int.log

test_StandardLibSuite/std_linked_list : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_linked_list" > test_reports/test_StandardLibSuite-std_linked_list.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_linked_list --report_sin=stdout >> test_reports/test_StandardLibSuite-std_linked_list.log

test_StandardLibSuite/std_vector : $(TEST_EXE)
	 @ echo "Run StandardLibSuite/std_vector" > test_reports/test_StandardLibSuite-std_vector.log
	 @ ./$(TEST_EXE) --run_test=StandardLibSuite/std_vector --report_sin=stdout >> test_reports/test_StandardLibSuite-std_vector.log

test_BugFixesSuite/while_bug : $(TEST_EXE)
	 @ echo "Run BugFixesSuite/while_bug" > test_reports/test_BugFixesSuite-while_bug.log
	 @ ./$(TEST_EXE) --run_test=BugFixesSuite/while_bug --report_sin=stdout >> test_reports/test_BugFixesSuite-while_bug.log

test_OptimizationSuite/parameter_propagation : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/parameter_propagation" > test_reports/test_OptimizationSuite-parameter_propagation.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/parameter_propagation --report_sin=stdout >> test_reports/test_OptimizationSuite-parameter_propagation.log

test_OptimizationSuite/global_cp : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cp" > test_reports/test_OptimizationSuite-global_cp.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/global_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cp.log

test_OptimizationSuite/global_offset_cp : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/global_offset_cp" > test_reports/test_OptimizationSuite-global_offset_cp.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/global_offset_cp --report_sin=stdout >> test_reports/test_OptimizationSuite-global_offset_cp.log

test_OptimizationSuite/remove_empty_functions : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_functions" > test_reports/test_OptimizationSuite-remove_empty_functions.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/remove_empty_functions --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_functions.log

test_OptimizationSuite/remove_empty_loops : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/remove_empty_loops" > test_reports/test_OptimizationSuite-remove_empty_loops.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/remove_empty_loops --report_sin=stdout >> test_reports/test_OptimizationSuite-remove_empty_loops.log

test_OptimizationSuite/invariant_code_motion : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/invariant_code_motion" > test_reports/test_OptimizationSuite-invariant_code_motion.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/invariant_code_motion --report_sin=stdout >> test_reports/test_OptimizationSuite-invariant_code_motion.log

test_OptimizationSuite/complete_loop_peeling : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling" > test_reports/test_OptimizationSuite-complete_loop_peeling.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling.log

test_OptimizationSuite/complete_loop_peeling_2 : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/complete_loop_peeling_2" > test_reports/test_OptimizationSuite-complete_loop_peeling_2.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/complete_loop_peeling_2 --report_sin=stdout >> test_reports/test_OptimizationSuite-complete_loop_peeling_2.log

test_OptimizationSuite/loop_unrolling : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unrolling" > test_reports/test_OptimizationSuite-loop_unrolling.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/loop_unrolling --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unrolling.log

test_OptimizationSuite/loop_unswitching : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/loop_unswitching" > test_reports/test_OptimizationSuite-loop_unswitching.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/loop_unswitching --report_sin=stdout >> test_reports/test_OptimizationSuite-loop_unswitching.log

test_OptimizationSuite/global_cse : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/global_cse" > test_reports/test_OptimizationSuite-global_cse.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/global_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-global_cse.log

test_OptimizationSuite/local_cse : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/local_cse" > test_reports/test_OptimizationSuite-local_cse.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/local_cse --report_sin=stdout >> test_reports/test_OptimizationSuite-local_cse.log

test_OptimizationSuite/cmov_opt : $(TEST_EXE)
	 @ echo "Run OptimizationSuite/cmov_opt" > test_reports/test_OptimizationSuite-cmov_opt.log
	 @ ./$(TEST_EXE) --run_test=OptimizationSuite/cmov_opt --report_sin=stdout >> test_reports/test_OptimizationSuite-cmov_opt.log

test_toString : $(TEST_EXE)
	 @ echo "Run toString" > test_reports/test_toString.log
	 @ ./$(TEST_EXE) --run_test=toString --report_sin=stdout >> test_reports/test_toString.log

test_toNumber : $(TEST_EXE)
	 @ echo "Run toNumber" > test_reports/test_toNumber.log
	 @ ./$(TEST_EXE) --run_test=toNumber --report_sin=stdout >> test_reports/test_toNumber.log

test_all: test_bb_entry_exit_count test_bb_iterators test_bb_new_bb test_bb_remove_bb test_bb_at test_ApplicationsSuite/applications_hangman test_SamplesSuite/samples_arrays test_SamplesSuite/samples_asm test_SamplesSuite/samples_assembly test_SamplesSuite/samples_bool test_SamplesSuite/samples_compound test_SamplesSuite/samples_concat test_SamplesSuite/samples_const test_SamplesSuite/samples_functions test_SamplesSuite/samples_float test_SamplesSuite/samples_little_float test_SamplesSuite/samples_casts test_SamplesSuite/samples_inc test_SamplesSuite/samples_includes test_SamplesSuite/samples_optimize test_SamplesSuite/samples_problem test_SamplesSuite/samples_identifiers test_SamplesSuite/samples_registers test_SamplesSuite/samples_structures test_SpecificSuite/addressof test_SpecificSuite/array_foreach_local test_SpecificSuite/array_foreach_global test_SpecificSuite/array_foreach_param_local test_SpecificSuite/array_foreach_param_global test_SpecificSuite/array_foreach_param_param test_SpecificSuite/arrays_in_struct test_SpecificSuite/char_type test_SpecificSuite/char_at test_SpecificSuite/ctor_dtor_heap test_SpecificSuite/ctor_dtor_stack test_SpecificSuite/copy_constructors test_SpecificSuite/casts test_SpecificSuite/compound test_SpecificSuite/delete_any test_SpecificSuite/if_ test_SpecificSuite/includes test_SpecificSuite/int_arrays test_SpecificSuite/string_arrays test_SpecificSuite/string_foreach test_SpecificSuite/string_pointers test_SpecificSuite/int_pointers test_SpecificSuite/bool_pointers test_SpecificSuite/cmove test_SpecificSuite/dynamic test_SpecificSuite/dynamic_arrays_in_struct test_SpecificSuite/dynamic_struct test_SpecificSuite/float_pointers test_SpecificSuite/struct_pointers test_SpecificSuite/member_pointers test_SpecificSuite/member_function_calls test_SpecificSuite/member_functions test_SpecificSuite/member_functions_param_stack test_SpecificSuite/memory test_SpecificSuite/pass_member_by_value test_SpecificSuite/ternary test_SpecificSuite/while_ test_SpecificSuite/do_while_ test_SpecificSuite/defaults test_SpecificSuite/float_1 test_SpecificSuite/float_2 test_SpecificSuite/for_ test_SpecificSuite/foreach_ test_SpecificSuite/globals_ test_SpecificSuite/inc test_SpecificSuite/void_ test_SpecificSuite/return_string test_SpecificSuite/return_by_value test_SpecificSuite/return_int test_SpecificSuite/return_pointers test_SpecificSuite/pointer_arrays test_SpecificSuite/recursive_functions test_SpecificSuite/single_inheritance test_SpecificSuite/math test_SpecificSuite/builtin test_SpecificSuite/assign_value test_SpecificSuite/println test_SpecificSuite/prints test_SpecificSuite/structures test_SpecificSuite/struct_member_pointers test_SpecificSuite/struct_array test_SpecificSuite/switch_ test_SpecificSuite/switch_string test_SpecificSuite/nested test_SpecificSuite/args test_TemplateSuite/class_templates test_TemplateSuite/function_templates test_TemplateSuite/member_function_templates test_CompilationErrorsSuite/params_assign test_CompilationErrorsSuite/wrong_print test_CompilationErrorsSuite/invalid_inheritance test_StandardLibSuite/std_lib_arrays_sum test_StandardLibSuite/std_lib_math_min test_StandardLibSuite/std_lib_math_max test_StandardLibSuite/std_lib_math_factorial test_StandardLibSuite/std_lib_math_pow test_StandardLibSuite/std_lib_str_equals test_StandardLibSuite/std_lib_string test_StandardLibSuite/std_lib_string_concat test_StandardLibSuite/std_lib_string_concat_int test_StandardLibSuite/std_linked_list test_StandardLibSuite/std_vector test_BugFixesSuite/while_bug test_OptimizationSuite/parameter_propagation test_OptimizationSuite/global_cp test_OptimizationSuite/global_offset_cp test_OptimizationSuite/remove_empty_functions test_OptimizationSuite/remove_empty_loops test_OptimizationSuite/invariant_code_motion test_OptimizationSuite/complete_loop_peeling test_OptimizationSuite/complete_loop_peeling_2 test_OptimizationSuite/loop_unrolling test_OptimizationSuite/loop_unswitching test_OptimizationSuite/global_cse test_OptimizationSuite/local_cse test_OptimizationSuite/cmov_opt test_toString test_toNumber 
	 @ bash ./tools/test_report.sh

.PHONY: test_all test_bb_entry_exit_count test_bb_iterators test_bb_new_bb test_bb_remove_bb test_bb_at test_ApplicationsSuite/applications_hangman test_SamplesSuite/samples_arrays test_SamplesSuite/samples_asm test_SamplesSuite/samples_assembly test_SamplesSuite/samples_bool test_SamplesSuite/samples_compound test_SamplesSuite/samples_concat test_SamplesSuite/samples_const test_SamplesSuite/samples_functions test_SamplesSuite/samples_float test_SamplesSuite/samples_little_float test_SamplesSuite/samples_casts test_SamplesSuite/samples_inc test_SamplesSuite/samples_includes test_SamplesSuite/samples_optimize test_SamplesSuite/samples_problem test_SamplesSuite/samples_identifiers test_SamplesSuite/samples_registers test_SamplesSuite/samples_structures test_SpecificSuite/addressof test_SpecificSuite/array_foreach_local test_SpecificSuite/array_foreach_global test_SpecificSuite/array_foreach_param_local test_SpecificSuite/array_foreach_param_global test_SpecificSuite/array_foreach_param_param test_SpecificSuite/arrays_in_struct test_SpecificSuite/char_type test_SpecificSuite/char_at test_SpecificSuite/ctor_dtor_heap test_SpecificSuite/ctor_dtor_stack test_SpecificSuite/copy_constructors test_SpecificSuite/casts test_SpecificSuite/compound test_SpecificSuite/delete_any test_SpecificSuite/if_ test_SpecificSuite/includes test_SpecificSuite/int_arrays test_SpecificSuite/string_arrays test_SpecificSuite/string_foreach test_SpecificSuite/string_pointers test_SpecificSuite/int_pointers test_SpecificSuite/bool_pointers test_SpecificSuite/cmove test_SpecificSuite/dynamic test_SpecificSuite/dynamic_arrays_in_struct test_SpecificSuite/dynamic_struct test_SpecificSuite/float_pointers test_SpecificSuite/struct_pointers test_SpecificSuite/member_pointers test_SpecificSuite/member_function_calls test_SpecificSuite/member_functions test_SpecificSuite/member_functions_param_stack test_SpecificSuite/memory test_SpecificSuite/pass_member_by_value test_SpecificSuite/ternary test_SpecificSuite/while_ test_SpecificSuite/do_while_ test_SpecificSuite/defaults test_SpecificSuite/float_1 test_SpecificSuite/float_2 test_SpecificSuite/for_ test_SpecificSuite/foreach_ test_SpecificSuite/globals_ test_SpecificSuite/inc test_SpecificSuite/void_ test_SpecificSuite/return_string test_SpecificSuite/return_by_value test_SpecificSuite/return_int test_SpecificSuite/return_pointers test_SpecificSuite/pointer_arrays test_SpecificSuite/recursive_functions test_SpecificSuite/single_inheritance test_SpecificSuite/math test_SpecificSuite/builtin test_SpecificSuite/assign_value test_SpecificSuite/println test_SpecificSuite/prints test_SpecificSuite/structures test_SpecificSuite/struct_member_pointers test_SpecificSuite/struct_array test_SpecificSuite/switch_ test_SpecificSuite/switch_string test_SpecificSuite/nested test_SpecificSuite/args test_TemplateSuite/class_templates test_TemplateSuite/function_templates test_TemplateSuite/member_function_templates test_CompilationErrorsSuite/params_assign test_CompilationErrorsSuite/wrong_print test_CompilationErrorsSuite/invalid_inheritance test_StandardLibSuite/std_lib_arrays_sum test_StandardLibSuite/std_lib_math_min test_StandardLibSuite/std_lib_math_max test_StandardLibSuite/std_lib_math_factorial test_StandardLibSuite/std_lib_math_pow test_StandardLibSuite/std_lib_str_equals test_StandardLibSuite/std_lib_string test_StandardLibSuite/std_lib_string_concat test_StandardLibSuite/std_lib_string_concat_int test_StandardLibSuite/std_linked_list test_StandardLibSuite/std_vector test_BugFixesSuite/while_bug test_OptimizationSuite/parameter_propagation test_OptimizationSuite/global_cp test_OptimizationSuite/global_offset_cp test_OptimizationSuite/remove_empty_functions test_OptimizationSuite/remove_empty_loops test_OptimizationSuite/invariant_code_motion test_OptimizationSuite/complete_loop_peeling test_OptimizationSuite/complete_loop_peeling_2 test_OptimizationSuite/loop_unrolling test_OptimizationSuite/loop_unswitching test_OptimizationSuite/global_cse test_OptimizationSuite/local_cse test_OptimizationSuite/cmov_opt test_toString test_toNumber 
