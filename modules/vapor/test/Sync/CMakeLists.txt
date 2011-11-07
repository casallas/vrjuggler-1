add_executable(sync_testCond testCond.cpp)
target_link_libraries(sync_testCond vpr-${MAJOR_VER_NUMBER}_${MINOR_VER_NUMBER})

#TODO test is an infinite loop

#add_test(NAME VPR-Sync-testCond
#	COMMAND sync_testCond)
