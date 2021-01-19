#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


int main()
{
    std::cout << "Welcome to use MyCP tool." << std::endl;

    char source_file[] = "source_file.txt";
    char target_file[] = "target_file.txt";

    // open 一个文件。
    int source_fd = open(source_file, O_CREAT | O_RDWR);
    int target_fd = open(target_file, O_CREAT | O_RDWR);
    
    // 按行读取内容。
    char buff[10240];
    int len = 0;
    while( (len = read(source_fd, buff, sizeof(buff))) > 0 ){
        // printf("%s\n", buff);
        int write_result = write(target_fd, buff, len);
        std::cout << write_result << std::endl;
    }
    // open create 另一个文件。
    // 将 buff 中的内容复制到这个文件中去。
    // close1。
    close(source_fd);
    close(target_fd);
    // close2。
    return 0;
}

