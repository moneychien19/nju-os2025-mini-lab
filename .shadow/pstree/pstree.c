#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "pstree.h"

int main(int argc, char *argvp[]) {
  if (argc == 1) {
    return 0;
  }
  
  int* pids = get_all_pids();
  for (size_t i = 0; pids[i] != 0; i++) {
    ProcessInfo info;
    if (get_process_info(pids[i], &info) == 0) {
      printf("PID: %d, PPID: %d, COMM: %s\n", info.pid, info.ppid, info.comm);
    }
  }

  return 1;
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

  const char *ppid_str = end + 2;
  return atoi(ppid_str);
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