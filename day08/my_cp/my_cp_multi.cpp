#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>

int MAX = 5; // 指定线程数。

int main(int argc, char *argv[])
{
    if(argc != 3){
        printf("Please use right format, like: my_cp a_file b_file.\n");
        exit(1);
    }

    int source_fd = open(argv[1], O_RDWR);
    int target_fd = open(argv[2], O_CREAT | O_RDWR);
    if(source_fd < 0 || target_fd < 0){
        printf("Open ERROR.");
        exit(1);
    }

    struct stat sbuf; // 文件属性的结构体。
    int ret_fstat = fstat(source_fd, &sbuf);
    if (ret_fstat != 0){
        printf("fstat error");
        exit(1);
    }
    int len = sbuf.st_size; // 文件长度。

    // 根据文件大小拓展目标文件。
    int ret_truncate = ftruncate(target_fd, len);
    if(ret_truncate != 0){
        printf("ftruncate error");
        exit(1);
    }

    // 在源文件和目标文件上使用进程间通信。
    char *ptr_source_mmap = NULL, *ptr_target_mmap = NULL;
    ptr_source_mmap = (char *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, source_fd, 0);
    ptr_target_mmap = (char *)mmap(NULL, len, PROT_READ|PROT_WRITE, MAP_SHARED, target_fd, 0);
    if(ptr_source_mmap == MAP_FAILED || ptr_target_mmap == MAP_FAILED){
        perror("mmap error");
        exit(1);
    }

    int size_per_process = len / MAX;
    int surplus_size = len % MAX;
    printf("len = %d\n", len);
    printf("size_per_process = %d\n", size_per_process);
    printf("surplus_size = %d\n", surplus_size);

    // 注意这里目的端和源端，不要想当然。
    // memcpy(ptr_target_mmap, ptr_source_mmap, len);

    for (int i = 0; i < MAX; i++){
        int start_location = i * size_per_process;
        int copy_size = 0;
        int result_fork = fork();
        if (result_fork < 0){
            printf("fork failed");
        }else if(result_fork == 0){ // 子进程，处理整齐的数据。
            if (i == MAX-1){ // 最后一个子进程应该处理不整齐的数据。
                copy_size = size_per_process + surplus_size;
            }else{ // 常规子进程。
                copy_size = size_per_process;
            }
            memcpy(ptr_target_mmap + start_location, ptr_source_mmap + start_location, copy_size); 
            break;
        }if(result_fork > 0){
            pid_t res_wait = wait(NULL);
            if(res_wait == -1){
                printf("wait failed.");
                exit(1);
            }
        }
    }

    munmap(ptr_source_mmap, len);
    munmap(ptr_target_mmap, len);

    close(source_fd);
    close(target_fd);

    return 0;
}

