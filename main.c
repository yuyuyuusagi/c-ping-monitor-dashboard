#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#define MAX_IP_LENGTH 100
#define MAX_TARGETS 50
#define INTERVAL_SECONDS 5

typedef struct {
    char ip[MAX_IP_LENGTH];
    int ng_count;
    int response_time;
} Target;

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void write_log(const char* ip, const char* status, int ng_count, int response_time) {
    FILE* log = fopen("ping_log.txt", "a");
    if (log == NULL) return;

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    fprintf(log, "%04d-%02d-%02d %02d:%02d:%02d, %s, %s, NG連続:%d, 応答:%dms\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec,
        ip, status, ng_count, response_time
    );

    fclose(log);
}

void write_csv(const char* ip, const char* status, int ng_count, int response_time) {
    FILE* csv = fopen("ping_report.csv", "a");

    if (csv == NULL) {
        return;
    }

    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    fprintf(csv, "%04d-%02d-%02d %02d:%02d:%02d,%s,%s,%d,%d\n",
        t->tm_year + 1900,
        t->tm_mon + 1,
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        ip,
        status,
        ng_count,
        response_time
    );

    fclose(csv);
}

void create_csv_header() {
    FILE* csv = fopen("ping_report.csv", "r");

    if (csv != NULL) {
        fclose(csv);
        return;
    }

    csv = fopen("ping_report.csv", "w");

    if (csv == NULL) {
        return;
    }

    fprintf(csv, "日時,IP,状態,連続NG回数,応答時間ms\n");
    fclose(csv);
}

int ping_check(const char* ip, int* response_time) {
    char command[256];
    char buffer[256];

    sprintf(command, "ping -n 1 -w 1000 %s", ip);

    FILE* fp = popen(command, "r");

    if (fp == NULL) {
        *response_time = -1;
        return 0;
    }

    int success = 0;
    *response_time = -1;

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char* time_pos = strstr(buffer, "time=");
        if (time_pos == NULL) {
            time_pos = strstr(buffer, "時間=");
        }

        if (time_pos != NULL) {
            char* num_pos = time_pos;

            while (*num_pos && (*num_pos < '0' || *num_pos > '9')) {
                num_pos++;
            }

            if (*num_pos >= '0' && *num_pos <= '9') {
                *response_time = atoi(num_pos);
                success = 1;
            }
        }

        if (strstr(buffer, "TTL=") != NULL) {
            success = 1;
        }
    }

    pclose(fp);
    return success;
}

int load_targets(Target targets[]) {
    FILE* file = fopen("targets.txt", "r");

    if (file == NULL) {
        set_color(12);
        printf("targets.txt が見つかりません。\n");
        set_color(7);
        return 0;
    }

    char line[MAX_IP_LENGTH];
    int count = 0;

    while (fgets(line, sizeof(line), file) != NULL && count < MAX_TARGETS) {
        line[strcspn(line, "\n")] = '\0';

        if (strlen(line) == 0) {
            continue;
        }

        strcpy(targets[count].ip, line);
        targets[count].ng_count = 0;
        targets[count].response_time = -1;
        count++;
    }

    fclose(file);
    return count;
}

void monitor_once(Target targets[], int count) {
    set_color(11);
    printf("=====================================\n");
    printf("     C Ping Monitor Ver6.0\n");
    printf("=====================================\n\n");
    set_color(7);

    for (int i = 0; i < count; i++) {
        printf("%-18s ", targets[i].ip);

        int response_time = -1;

        if (ping_check(targets[i].ip, &response_time)) {
            targets[i].ng_count = 0;
            targets[i].response_time = response_time;

            set_color(10);
            if (response_time >= 0) {
                printf("[ OK ] %dms\n", response_time);
            }
            else {
                printf("[ OK ] 応答時間不明\n");
            }
            set_color(7);

            write_log(targets[i].ip, "OK", targets[i].ng_count, targets[i].response_time);
            write_csv(targets[i].ip, "OK", targets[i].ng_count, targets[i].response_time);
        }
        else {
            targets[i].ng_count++;
            targets[i].response_time = -1;

            set_color(12);
            printf("[ NG ] 連続NG: %d回", targets[i].ng_count);

            if (targets[i].ng_count >= 3) {
                set_color(14);
                printf("  ALERT!");
            }

            printf("\n");
            set_color(7);

            write_log(targets[i].ip, "NG", targets[i].ng_count, targets[i].response_time);
            write_csv(targets[i].ip, "NG", targets[i].ng_count, targets[i].response_time);
        }
    }
}

int main() {
    Target targets[MAX_TARGETS];

    create_csv_header();

    int target_count = load_targets(targets);

    if (target_count == 0) {
        return 1;
    }

    while (1) {
        system("cls");

        monitor_once(targets, target_count);

        set_color(8);
        printf("\n%d秒後に再監視します... 終了: Ctrl + C\n", INTERVAL_SECONDS);
        set_color(7);

        Sleep(INTERVAL_SECONDS * 1000);
    }

    return 0;
}