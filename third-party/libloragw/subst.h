#define open(file, flags, ...) open_c(file, flags)
#define close(fd) close_c(fd)
#define printf(args...) printf_c(args)
#define fprintf(fd, args...) printf_c(args)
