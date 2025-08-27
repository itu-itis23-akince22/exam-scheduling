// Final cleaned schedule.c
#include "exam.h"
#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ----------------- Helpers -----------------
static struct Day *find_day(struct Schedule *schedule, const char *name)
{
    if (!schedule || !schedule->head || !name)
        return NULL;
    struct Day *d = schedule->head;
    for (int i = 0; i < 7; ++i)
    {
        if (strcmp(d->dayName, name) == 0)
            return d;
        d = d->nextDay;
    }
    return NULL;
}

static int is_valid_window(int startTime, int endTime)
{
    if (endTime <= startTime)
        return 0;
    if (startTime < 8 || endTime > 20)
        return 0;
    if ((endTime - startTime) > 3)
        return 0;
    return 1;
}

static int has_conflict(struct Day *day, int start, int end, struct Exam *skip)
{
    for (struct Exam *e = day->examList; e; e = e->next)
    {
        if (e == skip)
            continue;
        if (start < e->endTime && end > e->startTime)
            return 1;
    }
    return 0;
}

static void insert_exam_sorted(struct Day *day, struct Exam *node)
{
    if (!day->examList || node->startTime < day->examList->startTime)
    {
        node->next = day->examList;
        day->examList = node;
        return;
    }
    struct Exam *cur = day->examList;
    while (cur->next && cur->next->startTime <= node->startTime)
        cur = cur->next;
    node->next = cur->next;
    cur->next = node;
}

// ----------------- Core API -----------------
struct Schedule *CreateSchedule(void)
{
    const char *days[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    struct Schedule *s = (struct Schedule *)malloc(sizeof *s);
    if (!s)
        return NULL;
    s->head = NULL;
    struct Day *first = NULL, *prev = NULL;
    for (int i = 0; i < 7; ++i)
    {
        struct Day *d = (struct Day *)malloc(sizeof *d);
        if (!d)
        {
            if (first)
            {
                struct Day *c = first;
                for (int k = 0; k < 7; ++k)
                {
                    struct Exam *ex = c->examList;
                    while (ex)
                    {
                        struct Exam *nx = ex->next;
                        free(ex);
                        ex = nx;
                    }
                    struct Day *nxDay = c->nextDay;
                    free(c);
                    if (nxDay == first)
                        break;
                    c = nxDay;
                }
            }
            free(s);
            return NULL;
        }
        strncpy(d->dayName, days[i], MAX_DAY_NAME_LEN - 1);
        d->dayName[MAX_DAY_NAME_LEN - 1] = '\0';
        d->examList = NULL;
        d->nextDay = NULL;
        if (!first)
            first = d;
        else
            prev->nextDay = d;
        prev = d;
    }
    if (prev)
        prev->nextDay = first; // circular
    s->head = first;
    return s;
}

int AddExamToSchedule(struct Schedule *schedule, const char *day, int startTime, int endTime, const char *courseCode)
{
    if (!schedule || !schedule->head || !day || !courseCode)
    {
        printf("Invalid exam.\n");
        return 3;
    }
    if (!is_valid_window(startTime, endTime))
    {
        printf("Invalid exam.\n");
        return 3;
    }
    struct Day *d = find_day(schedule, day);
    if (!d)
    {
        printf("Invalid exam.\n");
        return 3;
    }

    if (has_conflict(d, startTime, endTime, NULL))
    {
        int dur = endTime - startTime;
        int t = endTime;
        while (t + dur <= 20 && has_conflict(d, t, t + dur, NULL))
            t++;
        if (t + dur <= 20 && !has_conflict(d, t, t + dur, NULL))
        {
            struct Exam *e = CreateExam(t, t + dur, courseCode);
            if (!e)
            {
                printf("Schedule full. Exam cannot be added.\n");
                return 2;
            }
            insert_exam_sorted(d, e);
            printf("%s exam added to %s at time %d to %d due to conflict.\n", courseCode, d->dayName, t, t + dur);
            return 1;
        }
        struct Day *cur = d->nextDay;
        for (int i = 1; i < 7; ++i)
        {
            if (!has_conflict(cur, startTime, endTime, NULL))
            {
                struct Exam *e = CreateExam(startTime, endTime, courseCode);
                if (!e)
                {
                    printf("Schedule full. Exam cannot be added.\n");
                    return 2;
                }
                insert_exam_sorted(cur, e);
                printf("%s exam added to %s at time %d to %d due to conflict.\n", courseCode, cur->dayName, startTime, endTime);
                return 1;
            }
            cur = cur->nextDay;
        }
        printf("Schedule full. Exam cannot be added.\n");
        return 2;
    }

    struct Exam *e = CreateExam(startTime, endTime, courseCode);
    if (!e)
    {
        printf("Schedule full. Exam cannot be added.\n");
        return 2;
    }
    insert_exam_sorted(d, e);
    printf("%s exam added to %s at time %d to %d without conflict.\n", courseCode, d->dayName, startTime, endTime);
    return 0;
}

int RemoveExamFromSchedule(struct Schedule *schedule, const char *day, int startTime)
{
    if (!schedule || !schedule->head || !day)
    {
        printf("Exam could not be found.\n");
        return 1;
    }
    struct Day *d = find_day(schedule, day);
    if (!d)
    {
        printf("Exam could not be found.\n");
        return 1;
    }
    struct Exam *prev = NULL, *cur = d->examList;
    while (cur && cur->startTime < startTime)
    {
        prev = cur;
        cur = cur->next;
    }
    if (!cur || cur->startTime != startTime)
    {
        printf("Exam could not be found.\n");
        return 1;
    }
    if (prev)
        prev->next = cur->next;
    else
        d->examList = cur->next;
    free(cur);
    printf("Exam removed successfully.\n");
    return 0;
}

int UpdateExam(struct Schedule *schedule, const char *oldDay, int oldStartTime, const char *newDay, int newStartTime, int newEndTime)
{
    if (!schedule || !schedule->head || !oldDay || !newDay)
    {
        printf("Exam could not be found.\n");
        return 2;
    }
    if (!is_valid_window(newStartTime, newEndTime))
    {
        printf("Invalid exam.\n");
        return 3;
    }
    struct Day *src = find_day(schedule, oldDay);
    if (!src)
    {
        printf("Exam could not be found.\n");
        return 2;
    }
    struct Exam *prev = NULL, *cur = src->examList;
    while (cur && cur->startTime < oldStartTime)
    {
        prev = cur;
        cur = cur->next;
    }
    if (!cur || cur->startTime != oldStartTime)
    {
        printf("Exam could not be found.\n");
        return 2;
    }
    struct Day *dst = find_day(schedule, newDay);
    if (!dst)
    {
        printf("Update unsuccessful.\n");
        return 1;
    }
    if (dst == src && cur->startTime == newStartTime && cur->endTime == newEndTime)
    {
        printf("Update successful.\n");
        return 0;
    }
    if (has_conflict(dst, newStartTime, newEndTime, (dst == src ? cur : NULL)))
    {
        printf("Update unsuccessful.\n");
        return 1;
    }
    if (prev)
        prev->next = cur->next;
    else
        src->examList = cur->next;
    cur->startTime = newStartTime;
    cur->endTime = newEndTime;
    cur->next = NULL;
    insert_exam_sorted(dst, cur);
    printf("Update successful.\n");
    return 0;
}

int ClearDay(struct Schedule *schedule, const char *day)
{
    if (!schedule || !schedule->head || !day)
    {
        printf("%s is already clear.\n", day ? day : "(Day)");
        return 1;
    }
    struct Day *src = find_day(schedule, day);
    if (!src)
    {
        printf("%s is already clear.\n", day);
        return 1;
    }
    if (!src->examList)
    {
        printf("%s is already clear.\n", src->dayName);
        return 1;
    }

    // Destination day: next day in circular list
    struct Day *dst = src->nextDay;
    int baseline = 8; // earliest hour to try for first relocated exam

    struct Exam *e = src->examList;
    while (e)
    {
        struct Exam *next = e->next; // keep next pointer because we'll detach e
        int duration = e->endTime - e->startTime;
        int placed = 0;
        for (int start = baseline; start + duration <= 20; ++start)
        {
            if (!has_conflict(dst, start, start + duration, NULL))
            {
                // Place exam here
                e->startTime = start;
                e->endTime = start + duration;
                e->next = NULL;
                insert_exam_sorted(dst, e);
                baseline = e->endTime; // next relocated exam cannot start before this ends
                placed = 1;
                break;
            }
        }
        if (!placed)
        {
            // Could not relocate this (and remaining) exams within the day; abort relocation.
            // Free remaining exams (including current) to clear the source day.
            while (e)
            {
                struct Exam *nx = e->next;
                free(e);
                e = nx;
            }
            src->examList = NULL;
            printf("Schedule full. Exams from %s could not be relocated.\n", src->dayName);
            return 2;
        }
        e = next;
    }

    // Source day cleared (all nodes moved)
    src->examList = NULL;
    printf("%s is cleared, exams relocated.\n", src->dayName);
    return 0;
}

void DeleteSchedule(struct Schedule *schedule)
{
    if (!schedule)
    {
        printf("Schedule is cleared.\n");
        return;
    }
    if (schedule->head)
    {
        struct Day *cur = schedule->head;
        for (int i = 0; i < 7; ++i)
        {
            struct Exam *e = cur->examList;
            while (e)
            {
                struct Exam *nx = e->next;
                free(e);
                e = nx;
            }
            cur->examList = NULL;
            struct Day *nd = cur->nextDay;
            free(cur);
            cur = nd;
        }
    }
    schedule->head = NULL; // leave schedule allocated as tests expect
    printf("Schedule is cleared.\n");
}

// ----------------- File I/O -----------------
static void chomp(char *s)
{
    size_t n = strlen(s);
    while (n && (s[n - 1] == '\n' || s[n - 1] == '\r'))
        s[--n] = '\0';
}
static void trim(char *s)
{
    char *p = s;
    while (*p && isspace((unsigned char)*p))
        ++p;
    if (p != s)
        memmove(s, p, strlen(p) + 1);
    size_t n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1]))
        s[--n] = '\0';
}
static int is_day_name(const char *s)
{
    const char *D[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
    for (int i = 0; i < 7; i++)
        if (strcmp(s, D[i]) == 0)
            return 1;
    return 0;
}

int ReadScheduleFromFile(struct Schedule *schedule, const char *filename)
{
    if (!schedule || !schedule->head || !filename)
        return -1;
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        printf("Input File can not be opened!\n");
        return -1;
    }
    char line[256];
    char curDay[32] = "";
    int loaded = 0;
    while (fgets(line, sizeof line, f))
    {
        chomp(line);
        trim(line);
        if (!*line)
            continue;
        if (is_day_name(line))
        {
            strncpy(curDay, line, sizeof curDay - 1);
            curDay[sizeof curDay - 1] = '\0';
            continue;
        }
        if (!*curDay)
            continue;
        int sT, eT;
        char code[MAX_COURSE_CODE_LEN];
        if (sscanf(line, "%d %d %49s", &sT, &eT, code) == 3)
        {
            int r = AddExamToSchedule(schedule, curDay, sT, eT, code);
            if (r != 2)
                loaded++; // success or relocated
        }
    }
    fclose(f);
    return loaded;
}

int WriteScheduleToFile(struct Schedule *schedule, const char *filename)
{
    if (!schedule || !schedule->head || !filename)
        return -1;
    FILE *f = fopen(filename, "w");
    if (!f)
    {
        printf("Output File can not be opened!\n");
        return -1;
    }
    struct Day *d = schedule->head;
    for (int i = 0; i < 7; i++)
    {
        fprintf(f, "%s\n", d->dayName);
        for (struct Exam *e = d->examList; e; e = e->next)
            fprintf(f, "%d %d %s\n", e->startTime, e->endTime, e->courseCode);
        fprintf(f, "\n");
        d = d->nextDay;
    }
    fclose(f);
    return 1;
}
