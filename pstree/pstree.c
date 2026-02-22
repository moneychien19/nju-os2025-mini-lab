#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "pstree.h"

static char* find_comm(const char *stat_buf);
static int find_ppid(const char *stat_buf);
static int is_all_digits(const char *s);
static int get_max_pid();
static void add_child(ProcessNode *parent, ProcessNode *child);
static int cmp_by_pid(const void *a, const void *b);

int main(int argc, char *argvp[]) {
  int show_pids = 0;
  int numeric_sort = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argvp[i], "--version") == 0 || strcmp(argvp[i], "-V") == 0) {
      printf("pstree version 1.0\n");
      return 0;
    } else if (strcmp(argvp[i], "--show-pids") == 0 || strcmp(argvp[i], "-p") == 0) {
      show_pids = 1;
    } else if (strcmp(argvp[i], "--numeric-sort") == 0 || strcmp(argvp[i], "-n") == 0) {
      numeric_sort = 1;
    } else {
      fprintf(stderr, "Invalid option: %s\nUsage: pstree [-p] [-n] [-V]\n", argvp[i]);
      return 1;
    }
  }

  int* pids = get_all_pids();
  ProcessNode *root = build_tree(pids);
  print_tree(root, show_pids, numeric_sort, "", 1);

  free(pids);
  return 0;
}

// 取得 /proc 下的所有 process id
int* get_all_pids() {
  DIR *d = opendir("/proc");
  if (!d) {
    return NULL;
  }

  size_t cap = 128;
  size_t n = 0;

  int *pids = malloc(cap * sizeof(int));

  struct dirent *entry;
  while ((entry = readdir(d)) != NULL) {
    const char *d_name = entry->d_name;
    if (!is_all_digits(d_name)) {
      continue;
    }

    char *end;
    long pid_long = strtol(entry->d_name, &end, 10);
    if (!end || *end) {
      continue;
    }

    int pid = (int)pid_long;
    if (n >= cap) {
      cap *= 2;
      int *new_pids = realloc(pids, cap * sizeof(int));
      if (!new_pids) {
        free(pids);
        closedir(d);
        return NULL;
      }
      pids = new_pids;
    }
    pids[n++] = pid;
    pids[n] = 0; // 以 0 作為結束標誌
  }

  closedir(d);
  return pids;
}

// 利用 /proc/[pid]/stat 取得進程的 ppid 和 comm (name)
int get_process_info(int pid, ProcessInfo *info) {
  char path[256];
  snprintf(path, sizeof(path), "/proc/%d/stat", pid);
  
  FILE *f = fopen(path, "r");
  if (!f) return -1;

  char buf[4096];
  if (!fgets(buf, sizeof(buf), f)) {
    fclose(f);
    return -1;
  }

  fclose(f);

  info->pid = pid;
  
  char *comm = find_comm(buf);
  if (!comm) return -1;
  strncpy(info->comm, comm, sizeof(info->comm) - 1);
  info->comm[sizeof(info->comm) - 1] = '\0';
  free(comm);
  
  info->ppid = find_ppid(buf);
  
  return 0;
}

ProcessNode* build_tree(int *pids) {
  int max_pid = get_max_pid();
  if (max_pid <= 0) {
    return NULL;
  }

  ProcessNode* nodes = malloc(sizeof(ProcessNode) * (max_pid + 1));
  if (!nodes) {
    return NULL;
  }
  memset(nodes, 0, sizeof(ProcessNode) * (max_pid + 1));

  // Initialize nodes by pid index
  for (size_t i = 0; pids[i] != 0; i++) {
    ProcessInfo info;
    if (get_process_info(pids[i], &info) == 0) {
      ProcessNode *node = &nodes[info.pid];
      node->info = info;
      node->children = NULL;
      node->child_count = 0;
      node->child_cap = 0;
    }
  }

  // Wire parent-child relationships
  for (size_t i = 0; pids[i] != 0; i++) {
    int pid = pids[i];
    ProcessNode *node = &nodes[pid];
    int ppid = node->info.ppid;
    if (ppid > 0 && nodes[ppid].info.pid == ppid) {
      add_child(&nodes[ppid], node);
    }
  }
  return &nodes[1];
}

void print_tree(ProcessNode *node, int show_pids, int numeric_sort, const char *prefix, int is_last) {
  if (!node) return;

  printf("%s", prefix);
  if (prefix[0] != '\0') {
    printf(is_last ? "`-- " : "+-- ");
  }
  if (show_pids) {
    printf("%s (%d)\n", node->info.comm, node->info.pid);
  } else {
    printf("%s\n", node->info.comm);
  }

  char new_prefix[256];
  snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "|   ");

  if (numeric_sort && node->child_count > 1) {
    qsort(node->children, node->child_count, sizeof(ProcessNode *), cmp_by_pid);
  }

  for (size_t i = 0; i < node->child_count; i++) {
    printf("%s|\n", new_prefix);
    print_tree(node->children[i], show_pids, numeric_sort, new_prefix, i == node->child_count - 1);
  }
}

static char* find_comm(const char *stat_buf) {
  const char *start = strchr(stat_buf, '(');
  const char *end = strrchr(stat_buf, ')');
  if (!start || !end || end <= start) {
    return NULL;
  }

  size_t len = end - start - 1;
  char *comm = malloc(len + 1);
  if (!comm) {
    return NULL;
  }
  
  strncpy(comm, start + 1, len);
  comm[len] = '\0';
  
  return comm;
}

static int find_ppid(const char *stat_buf) {
  const char *end = strrchr(stat_buf, ')');
  if (!end) {
    return -1;
  }

  int ppid;
  sscanf(end + 2, "%*c %d", &ppid);
  return ppid;
}

static int is_all_digits(const char *s) {
  if (!s || !*s) {
    return 0;
  }
  while (*s) {
    if (!isdigit(*s)) {
      return 0;
    }
    s++;
  }
  
  return 1;
}

static int get_max_pid() {
  FILE *f = fopen("/proc/sys/kernel/pid_max", "r");
  if (!f) {
    return -1;
  }
  
  int max_pid;
  if (fscanf(f, "%d", &max_pid) != 1) {
    fclose(f);
    return -1;
  }

  fclose(f);
  return max_pid;
}

static void add_child(ProcessNode *parent, ProcessNode *child) {
  if (parent->child_count >= parent->child_cap) {
    size_t new_cap = parent->child_cap == 0 ? 4 : parent->child_cap * 2;
    ProcessNode **new_children = realloc(parent->children, new_cap * sizeof(ProcessNode*));
    
    if (!new_children) {
      return;
    }

    parent->children = new_children;
    parent->child_cap = new_cap;
  }
  
  parent->children[parent->child_count++] = child;
}

static int cmp_by_pid(const void *a, const void *b) {
  ProcessNode *pa = *(ProcessNode **)a;
  ProcessNode *pb = *(ProcessNode **)b;
  return pa->info.pid - pb->info.pid;
}
