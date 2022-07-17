add_executable(kal_llvm Ast.cpp main.cpp)
target_link_libraries(kal_llvm ${REQUIRED_LLVM_LIBS})
target_include_directories(kal_llvm PUBLIC include)