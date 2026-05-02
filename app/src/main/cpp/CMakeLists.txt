cmake_minimum_required(VERSION 3.22.1)

project("jnidemo")

# --- Bibliothèque principale (fonctions JNI existantes) ---
add_library(
        native-lib
        SHARED
        native-lib.cpp)

# --- Bibliothèque de sécurité (nouveau fichier séparé) ---
add_library(
        security-lib
        SHARED
        security-lib.cpp)

# Récupère la lib Android log
find_library(log-lib log)

# Lie les deux bibliothèques à log
target_link_libraries(native-lib  ${log-lib})
target_link_libraries(security-lib ${log-lib})