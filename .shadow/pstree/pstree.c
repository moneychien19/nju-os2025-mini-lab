#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "pstree.h"

int main(int argc, char *argvp[]) {
  // if (argc == 1) {
  //   return 0;
  // }
  
  int* pids = get_all_pids();
  ProcessNode *root = build_tree(pids);
  print_tree(root, "", 1);

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

void print_tree(ProcessNode *node, const char *prefix, int is_last) {
  if (!node) return;

  printf("%s", prefix);
  if (prefix[0] != '\0') {
    printf(is_last ? "`-- " : "+-- ");
  }
  printf("%s\n", node->info.comm);
  
  char new_prefix[256];
  snprintf(new_prefix, sizeof(new_prefix), "%s%s", prefix, is_last ? "    " : "|   ");

  for (size_t i = 0; i < node->child_count; i++) {
    printf("%s|\n", new_prefix);
    print_tree(node->children[i], new_prefix, i == node->child_count - 1);
  }
}

char* find_comm(const char *stat_buf) {
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

int find_ppid(const char *stat_buf) {
  const char *end = strrchr(stat_buf, ')');
  if (!end) {
    return -1;
  }

  int ppid;
  sscanf(end + 2, "%*c %d", &ppid);
  return ppid;
}

int is_all_digits(const char *s) {
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

int get_max_pid() {
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

void add_child(ProcessNode *parent, ProcessNode *child) {
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