typedef struct {
  int pid;
  int ppid;
  char comm[256];
} ProcessInfo;

typedef struct {
  ProcessInfo info;
  struct ProcessNode **children;
  size_t child_count;
  size_t child_cap;
} ProcessNode;

int* get_all_pids();
int get_process_info(int pid, ProcessInfo *info);
ProcessNode* build_tree(int *pids);
void print_tree(ProcessNode *node, const char *prefix, int is_last);

char* find_comm(const char *stat_buf);
int find_ppid(const char *stat_buf);
int is_all_digits(const char *s);
int get_max_pid();
void add_child(ProcessNode *parent, ProcessNode *child);