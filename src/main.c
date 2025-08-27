#include "schedule.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    // Create an initial schedule for testing
    struct Schedule *schedule = CreateSchedule();

    // Reading and writing schedule from/to a file
    const char *inputFile = "/workspaces/Exam_Scheduling_System/src/exam_schedule_input.txt";
    const char *outputFile = "/workspaces/Exam_Scheduling_System/src/exam_schedule_output.txt";

    // Uncomment to read schedule from file (assuming the input file exists and is correctly formatted)
    // ReadScheduleFromFile(schedule, inputFile);

    // Example: Add exams to the schedule
    AddExamToSchedule(schedule, "Monday", 8, 9, "BLG113E");
    AddExamToSchedule(schedule, "Monday", 9, 10, "BLG223E");
    AddExamToSchedule(schedule, "Sunday", 11, 13, "BLG102E");
    AddExamToSchedule(schedule, "Wednesday", 9, 10, "BLG223E");

    // Example: Remove an exam from the schedule
    RemoveExamFromSchedule(schedule, "Wednesday", 9);

    // Example: Update an exam in the schedule
    UpdateExam(schedule, "Sunday", 11, "Wednesday", 13, 14); // Corrected time in the example

    // Example: Clear a day in the schedule
    ClearDay(schedule, "Monday");

    // Write schedule to file
    WriteScheduleToFile(schedule, outputFile);

    // Example: Delete the entire schedule
    DeleteSchedule(schedule);

    return 0;
}
