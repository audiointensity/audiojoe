@echo off
antlr4.bat joe.g4 && javac *.java && grun joe joefile -tree -tokens %*
