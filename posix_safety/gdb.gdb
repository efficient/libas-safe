def hook-run
han SIGALRM ignore
han SIGSEGV noprint
end
b main
comm
han SIGSEGV stop
end
