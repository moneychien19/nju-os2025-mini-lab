#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"

int main(int argc, char *argv[]) {
    // TODO: Implement this function
    if (argc == 1) {
        printUsage();
        return 0;
    }

    if (strcmp(argv[1], "--version") == 0) {
        if (argc != 2) {
            return 1;
        }
        showVersion();
        return 0;
    }

    char *filename = NULL;
    char *playerId = NULL;
    char *direction = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--map") == 0 || strcmp(argv[i], "-m") == 0) {
            filename = argv[++i];
        } else if (strcmp(argv[i], "--player") == 0 || strcmp(argv[i], "-p") == 0) {
            playerId = argv[++i];
        } else if (strcmp(argv[i], "--move") == 0 || strcmp(argv[i], "-mv") == 0) {
            direction = argv[++i];
        } else {
            return 1;
        }
    }

    if (filename != NULL && playerId != NULL) {
        Labyrinth labyrinth;
        bool loadMapSuccess = loadMap(&labyrinth, filename);
        if (!loadMapSuccess) {
            return 1;
        }
        if (!isValidPlayer(playerId[0])) {
            return 1;
        }

        for (int i = 0; i < labyrinth.rows; i++) {
            printf("%s\n", labyrinth.map[i]);
        }
    }

    return 0;
}

void printUsage() {
    printf("Usage:\n");
    printf("  labyrinth --map map.txt --player id\n");
    printf("  labyrinth -m map.txt -p id\n");
    printf("  labyrinth --map map.txt --player id --move direction\n");
    printf("  labyrinth --version\n");
}

bool isValidPlayer(char playerId) {
    if (playerId >= '0' && playerId <= '9') {
        return true;
    }

    return false;
}

bool loadMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    FILE *f = fopen(filename, "r");

    // 如果地图文件不存在或格式不正确，退出并返回错误码 1
    if (f == NULL) {
        return false;
    }

    // 如果缺少任何必需参数，退出并返回错误码 1
    
    char buffer[MAX_COLS + 2]; // +2 for newline and null terminator

    int row = 0;
    while (fgets(buffer, sizeof(buffer), f) != NULL && row < MAX_ROWS) {
        buffer[strcspn(buffer, "\n")] = '\0';
        strncpy(labyrinth->map[row], buffer, MAX_COLS);
        labyrinth->map[row][MAX_COLS - 1] = '\0'; // ensure null-terminated
        row++;
    }
    labyrinth->rows = row;
    labyrinth->cols = (row > 0) ? strlen(labyrinth->map[0]) : 0;

    fclose(f);

    // 如果迷宫过大，退出并返回错误码 1
    if (labyrinth->cols > MAX_COLS || labyrinth->rows > MAX_ROWS) {
        return false;
    }

    // 如果迷宫中的所有空地不连通，退出并返回错误码 1

    return true;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    // TODO: Implement this function
    Position pos = {-1, -1};
    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    // TODO: Implement this function
    Position pos = {-1, -1};
    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    // TODO: Implement this function
    return false;
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    // TODO: Implement this function
    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    return false;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    // TODO: Implement this function
}

bool isConnected(Labyrinth *labyrinth) {
    // TODO: Implement this function
    return false;
}

void showVersion() {
    printf("Labyrinth Game Version 1.0\n");   
}