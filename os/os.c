#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define RAM_SIZE 2048
#define PRIORITY_0_RAM 512
#define QUANTUM_TIME_2 8
#define QUANTUM_TIME_3 16

typedef struct {
    char id[5];
    int arrival_time;
    int priority;
    int burst_time;
    int ram;
    int cpu;
} Process;

typedef struct {
    Process processes[MAX_PROCESSES];
    int count;
} Queue;

// Function prototypes
void read_input_file(const char *filename, Queue *all_processes);
void schedule_processes(Queue *all_processes);
void print_queues(Queue *q1, Queue *q2, Queue *q3, Queue *q4, FILE *output_file);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    Queue all_processes = { .count = 0 };
    read_input_file(argv[1], &all_processes);
    schedule_processes(&all_processes);

    return 0;
}

void read_input_file(const char *filename, Queue *all_processes) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open input file");
        exit(1);
    }
    printf("Successfully opened input file: %s\n", filename);

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        Process p;
        if (sscanf(line, "%[^,],%d,%d,%d,%d,%d", p.id, &p.arrival_time, &p.priority, &p.burst_time, &p.ram, &p.cpu) != 6) {
            fprintf(stderr, "Failed to parse line: %s\n", line);
            continue;
        }
        all_processes->processes[all_processes->count++] = p;
    }
    printf("Successfully read %d processes from input file.\n", all_processes->count);

    fclose(file);
}

void schedule_processes(Queue *all_processes) {
    Queue queue_0 = { .count = 0 };
    Queue queue_1 = { .count = 0 };
    Queue queue_2 = { .count = 0 };
    Queue queue_3 = { .count = 0 };

    int ram_available = RAM_SIZE - PRIORITY_0_RAM;
    int ram_priority_0_used = 0;

    FILE *output_file = fopen("output.txt", "w");
    if (!output_file) {
        perror("Failed to open output file");
        exit(1);
    }
    printf("Successfully opened output file.\n");

    for (int i = 0; i < all_processes->count; i++) {
        Process p = all_processes->processes[i];

        if (p.priority == 0) {
            if (ram_priority_0_used + p.ram <= PRIORITY_0_RAM) {
                queue_0.processes[queue_0.count++] = p;
                ram_priority_0_used += p.ram;
                fprintf(output_file, "Process %s is queued to be assigned to CPU-1.\n", p.id);
            } else {
                fprintf(output_file, "Process %s cannot be queued to CPU-1 due to insufficient RAM.\n", p.id);
            }
        } else if (p.ram <= ram_available) {
            ram_available -= p.ram;
            if (p.priority == 1) {
                queue_1.processes[queue_1.count++] = p;
                fprintf(output_file, "Process %s is placed in the queue1 (priority-1) to be assigned to CPU-2.\n", p.id);
            } else if (p.priority == 2) {
                queue_2.processes[queue_2.count++] = p;
                fprintf(output_file, "Process %s is placed in the queue2 (priority-2) to be assigned to CPU-2.\n", p.id);
            } else if (p.priority == 3) {
                queue_3.processes[queue_3.count++] = p;
                fprintf(output_file, "Process %s is placed in the queue3 (priority-3) to be assigned to CPU-2.\n", p.id);
            }
        } else {
            fprintf(output_file, "Process %s cannot be queued due to insufficient RAM.\n", p.id);
        }
    }

    // Print the final queues
    print_queues(&queue_0, &queue_1, &queue_2, &queue_3, output_file);

    // Simulate scheduling and processing
    // CPU-1 with FCFS for queue_0
    fprintf(output_file, "\nCPU-1 (FCFS) Processing:\n");
    for (int i = 0; i < queue_0.count; i++) {
        fprintf(output_file, "Process %s is assigned to CPU-1.\n", queue_0.processes[i].id);
        fprintf(output_file, "Process %s is completed and terminated.\n", queue_0.processes[i].id);
    }

    // CPU-2 with different scheduling algorithms for queue_1, queue_2, and queue_3
    fprintf(output_file, "\nCPU-2 (SJF for priority-1) Processing:\n");
    // Sort queue_1 based on burst time for SJF
    for (int i = 0; i < queue_1.count - 1; i++) {
        for (int j = i + 1; j < queue_1.count; j++) {
            if (queue_1.processes[i].burst_time > queue_1.processes[j].burst_time) {
                Process temp = queue_1.processes[i];
                queue_1.processes[i] = queue_1.processes[j];
                queue_1.processes[j] = temp;
            }
        }
    }
    for (int i = 0; i < queue_1.count; i++) {
        fprintf(output_file, "Process %s is assigned to CPU-2.\n", queue_1.processes[i].id);
        fprintf(output_file, "Process %s is completed and terminated.\n", queue_1.processes[i].id);
    }

    // Round Robin scheduling for queue_2 (priority 2)
    fprintf(output_file, "\nCPU-2 (Round Robin for priority-2) Processing:\n");
    int quantum = QUANTUM_TIME_2;
    while (queue_2.count > 0) {
        Process p = queue_2.processes[0];
        fprintf(output_file, "Process %s is assigned to CPU-2.\n", p.id);
        if (p.burst_time > quantum) {
            p.burst_time -= quantum;
            fprintf(output_file, "Process %s run for %d time units and is queued again.\n", p.id, quantum);
            for (int i = 1; i < queue_2.count; i++) {
                queue_2.processes[i - 1] = queue_2.processes[i];
            }
            queue_2.processes[queue_2.count - 1] = p;
        } else {
            fprintf(output_file, "Process %s run for %d time units and is completed.\n", p.id, p.burst_time);
            for (int i = 1; i < queue_2.count; i++) {
                queue_2.processes[i - 1] = queue_2.processes[i];
            }
            queue_2.count--;
        }
    }

    // Round Robin scheduling for queue_3 (priority 3)
    fprintf(output_file, "\nCPU-2 (Round Robin for priority-3) Processing:\n");
    quantum = QUANTUM_TIME_3;
    while (queue_3.count > 0) {
        Process p = queue_3.processes[0];
        fprintf(output_file, "Process %s is assigned to CPU-2.\n", p.id);
        if (p.burst_time > quantum) {
            p.burst_time -= quantum;
            fprintf(output_file, "Process %s run for %d time units and is queued again.\n", p.id, quantum);
            for (int i = 1; i < queue_3.count; i++) {
                queue_3.processes[i - 1] = queue_3.processes[i];
            }
            queue_3.processes[queue_3.count - 1] = p;
        } else {
            fprintf(output_file, "Process %s run for %d time units and is completed.\n", p.id, p.burst_time);
            for (int i = 1; i < queue_3.count; i++) {
                queue_3.processes[i - 1] = queue_3.processes[i];
            }
            queue_3.count--;
        }
    }

    fclose(output_file);
    printf("Successfully written to output file.\n");
}

void print_queues(Queue *q1, Queue *q2, Queue *q3, Queue *q4, FILE *output_file) {
    printf("\nQueue status:\n");
    printf("CPU-1 queue (priority-0) (FCFS) : ");
    for (int i = 0; i < q1->count; i++) {
        printf("%s", q1->processes[i].id);
        if (i < q1->count - 1) printf("-");
    }
    printf("\nCPU-2 queue (priority-1) (SJF) : ");
    for (int i = 0; i < q2->count; i++) {
        printf("%s", q2->processes[i].id);
        if (i < q2->count - 1) printf("-");
    }
    printf("\nCPU-2 queue (priority-2) (RR-q8) : ");
    for (int i = 0; i < q3->count; i++) {
        printf("%s", q3->processes[i].id);
        if (i < q3->count - 1) printf("-");
    }
    printf("\nCPU-2 queue (priority-3) (RR-q16) : ");
    for (int i = 0; i < q4->count; i++) {
        printf("%s", q4->processes[i].id);
        if (i < q4->count - 1) printf("-");
    }
    printf("\n");

    // Print the same information to the output file
    fprintf(output_file, "\nQueue status:\n");
    fprintf(output_file, "CPU-1 queue (priority-0) (FCFS) : ");
    for (int i = 0; i < q1->count; i++) {
        fprintf(output_file, "%s", q1->processes[i].id);
        if (i < q1->count - 1) fprintf(output_file, "-");
    }
    fprintf(output_file, "\nCPU-2 queue (priority-1) (SJF) : ");
    for (int i = 0; i < q2->count; i++) {
        fprintf(output_file, "%s", q2->processes[i].id);
        if (i < q2->count - 1) fprintf(output_file, "-");
    }
    fprintf(output_file, "\nCPU-2 queue (priority-2) (RR-q8) : ");
    for (int i = 0; i < q3->count; i++) {
        fprintf(output_file, "%s", q3->processes[i].id);
        if (i < q3->count - 1) fprintf(output_file, "-");
    }
    fprintf(output_file, "\nCPU-2 queue (priority-3) (RR-q16) : ");
    for (int i = 0; i < q4->count; i++) {
        fprintf(output_file, "%s", q4->processes[i].id);
        if (i < q4->count - 1) fprintf(output_file, "-");
    }
    fprintf(output_file, "\n");
}
