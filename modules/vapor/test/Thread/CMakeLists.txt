add_executable(thread_stlTest stlTest.cpp)
target_link_libraries(thread_stlTest vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
add_test(NAME VPR-Thread-stlTest
	COMMAND thread_stlTest)

add_executable(thread_test test.cpp)
target_link_libraries(thread_test vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
# TODO test is an infinite loop
#add_test(NAME VPR-Thread-test
#	COMMAND thread_test)

add_executable(thread_testPool testPool.cpp)
target_link_libraries(thread_testPool vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
add_test(NAME VPR-Thread-testPool
	COMMAND thread_testPool)

add_executable(thread_testSelfPerformance testSelfPerformance.cpp)
target_link_libraries(thread_testSelfPerformance vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
add_test(NAME VPR-Thread-testSelfPerformance
	COMMAND thread_testSelfPerformance 5)

add_executable(thread_testTSD_Performance testTSD_Performance.cpp)
target_link_libraries(thread_testTSD_Performance vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})
add_test(NAME VPR-Thread-testTSD_Performance
	COMMAND thread_testTSD_Performance 5)
