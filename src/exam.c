#include "../include/exam.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to create a new exam
struct Exam *CreateExam(int startTime, int endTime, const char *courseCode)
{
    struct Exam *newExam = (struct Exam *)malloc(sizeof(struct Exam));
    if (!newExam)
        return NULL;

    newExam->startTime = startTime;
    newExam->endTime = endTime;
    strcpy(newExam->courseCode, courseCode);
    newExam->next = NULL;

    return newExam;
}

// Helper function to print an exam
void PrintExam(struct Exam *exam)
{
    if (!exam)
        return;
    printf("%s  %d %d\n", exam->courseCode, exam->startTime, exam->endTime);
}
