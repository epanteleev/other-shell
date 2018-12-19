project(origin_shell)
set(SOURCES execute.c
	execute.h
	main.c
	parser.c
	process_list.c
	process_list.h
	signal_handler.c
	execute.h
	parser.h
	process_list.h
        common.h)
add_executable(${PROJECT_NAME} ${SOURCES})
