typedef struct {
  int pid;
  int ppid;
  char comm[256];
} ProcessInfo;

typedef struct ProcessNode {
  ProcessInfo info;
  struct ProcessNode **children;
  size_t child_count;
  size_t child_cap;
} ProcessNode;

int* get_all_pids();
int get_process_info(int pid, ProcessInfo *info);
ProcessNode* build_tree(int *pids);
void print_tree(ProcessNode *node, int show_pids, int numeric_sort, const char *prefix, int is_last);
