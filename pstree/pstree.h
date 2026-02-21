typedef struct {
  int pid;
  int ppid;
  char comm[256];
} ProcessInfo;

int* get_all_pids();
int get_process_info(int pid, ProcessInfo *info);
char* find_comm(const char *stat_buf);
int find_ppid(const char *stat_buf);
int is_all_digits(const char *s);