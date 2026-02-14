#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <testkit.h>
#include "labyrinth.h"

int main(int argc, char *argv[]) {
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
    Labyrinth labyrinth;

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
        
        if (direction != NULL) {
            if (!movePlayer(&labyrinth, playerId[0], direction)) {
                return 1;
            }
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
    if (!isConnected(labyrinth)) {
        return false;
    }

    return true;
}

Position findPlayer(Labyrinth *labyrinth, char playerId) {
    Position pos = {-1, -1};

    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] == playerId) {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }

    return pos;
}

Position findFirstEmptySpace(Labyrinth *labyrinth) {
    Position pos = {-1, -1};

    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] == '.') {
                pos.row = i;
                pos.col = j;
                return pos;
            }
        }
    }

    return pos;
}

bool isEmptySpace(Labyrinth *labyrinth, int row, int col) {
    if (row < 0 || row >= labyrinth->rows || col < 0 || col >= labyrinth->cols) {
        return false;
    }

    return labyrinth->map[row][col] == '.';
}

bool movePlayer(Labyrinth *labyrinth, char playerId, const char *direction) {
    Position position = findPlayer(labyrinth, playerId);

    if (position.row == -1 || position.col == -1) {
        position = findFirstEmptySpace(labyrinth);
        labyrinth->map[position.row][position.col] = playerId;
    }

    if (strcmp(direction, "up") == 0) {
        if (position.row > 0 && isEmptySpace(labyrinth, position.row - 1, position.col)) {
            labyrinth->map[position.row][position.col] = '.';
            labyrinth->map[position.row - 1][position.col] = playerId;
            return true;
        }
    } else if (strcmp(direction, "down") == 0) {
        if (position.row < labyrinth->rows - 1 && isEmptySpace(labyrinth, position.row + 1, position.col)) {
            labyrinth->map[position.row][position.col] = '.';
            labyrinth->map[position.row + 1][position.col] = playerId;
            return true;
        }
    } else if (strcmp(direction, "left") == 0) {
        if (position.col > 0 && isEmptySpace(labyrinth, position.row, position.col - 1)) {
            labyrinth->map[position.row][position.col] = '.';
            labyrinth->map[position.row][position.col - 1] = playerId;
            return true;
        }
    } else if (strcmp(direction, "right") == 0) {
        if (position.col < labyrinth->cols - 1 && isEmptySpace(labyrinth, position.row, position.col + 1)) {
            labyrinth->map[position.row][position.col] = '.';
            labyrinth->map[position.row][position.col + 1] = playerId;
            return true;
        }
    }

    return false;
}

bool saveMap(Labyrinth *labyrinth, const char *filename) {
    // TODO: Implement this function
    return false;
}

// Check if all empty spaces are connected using DFS
void dfs(Labyrinth *labyrinth, int row, int col, bool visited[MAX_ROWS][MAX_COLS]) {
    Position directions[4] = {
        { -1, 0 }, // up
        { 1, 0 },  // down
        { 0, -1 }, // left
        { 0, 1 }   // right
    };

    for (int i = 0; i < 4; i++) {
        int newRow = row + directions[i].row;
        int newCol = col + directions[i].col;

        if (newRow >= 0 && newRow < labyrinth->rows && newCol >= 0 && newCol < labyrinth->cols) {
            if (!visited[newRow][newCol] && labyrinth->map[newRow][newCol] != '#') {
                visited[newRow][newCol] = true;
                dfs(labyrinth, newRow, newCol, visited);
            }
        }
    }
}

bool isConnected(Labyrinth *labyrinth) {
    bool visited[MAX_ROWS][MAX_COLS] = {{ false }};

    Position start = findFirstEmptySpace(labyrinth);
    if (start.row == -1 || start.col == -1) {
        return false;
    }
    
    dfs(labyrinth, start.row, start.col, visited);

    for (int i = 0; i < labyrinth->rows; i++) {
        for (int j = 0; j < labyrinth->cols; j++) {
            if (labyrinth->map[i][j] != '#' && !visited[i][j]) {
                return false;
            }
        }
    }

    return true;
}

void showVersion() {
    printf("Labyrinth Game Version 1.0\n");   
}