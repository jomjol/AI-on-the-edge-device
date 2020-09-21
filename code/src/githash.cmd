@echo off
FOR /F "tokens=* USEBACKQ" %%F IN (`git describe --dirty --always --tags`) DO (
SET var=%%F
)
ECHO #ifndef GITHASH_H > githash.h
ECHO #define GITHASH_H >> githash.h
ECHO const std::string kGitHash = "%var%"; >> githash.h
ECHO #endif // GITHASH_H >> githash.h