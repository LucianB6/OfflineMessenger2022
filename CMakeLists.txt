cmake_minimum_required(VERSION 3.21)
project(ProiectRetele2022 C)

set(CMAKE_C_STANDARD 99)

add_executable(server
               server.c
        )

add_executable(client
               client.c)

add_executable(test_table_users users.c)

add_executable(test_table_mess mesaje.c)

target_link_libraries(server pthread sqlite3)

target_link_libraries(test_table_users sqlite3)
target_link_libraries(test_table_mess sqlite3)

