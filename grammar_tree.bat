@echo off
REM antlr4.bat joe.g4 && javac *.java && grun joe joefile -tree -tokens %*
antlr4.bat joe.g4 && javac joe*.java grammarTree_joe.java && java grammarTree_joe %*
