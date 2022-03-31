#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#define NCAT 1 // файл, не являющийся каталогом
#define CAT 2 // каталог
#define NREAD 3 // каталог, недоступный для чтения
#define NSTAT 4 // файл, информацию о котором нельзя получить с помощью stat

// Тип функции, которая будет вызываться для каждого встреченного файла
typedef int Handler(const char *,const struct stat *, int);

static Handler counter;
static int make_tree(const char *filename, int depth, Handler *);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, nTotal;

int main(int argc, char * argv[])
{
    int ret = -1;
    if (argc != 2)
    {
        printf("ERROR, wrong arguments.\nUse: ./app <dir>\n");
        exit(-1);
    }

    ret = make_tree(argv[1], 0, counter); //выполняет всю работу

    nTotal = nreg + ndir +  nblk + nchr +  nfifo + nslink + nsock;

    if (nTotal == 0)
        nTotal = 1; // во избежание деления на 0

    printf("_______________________________\nSummary:\n\n");
    printf("Common files:\t%7ld, %5.2f %%\n", nreg, nreg*100.0/nTotal);
    printf("Catalogs:\t%7ld, %5.2f %%\n", ndir, ndir*100.0/nTotal);
    printf("Block devices:\t%7ld, %5.2f %%\n", nblk, nblk*100.0/nTotal);
    printf("Char devices:\t%7ld, %5.2f %%\n", nchr, nchr*100.0/nTotal);
    printf("FIFOs:\t\t%7ld, %5.2f %%\n", nfifo, nfifo*100.0/nTotal);
    printf("Symbolic links:\t%7ld, %5.2f %%\n", nslink, nslink*100.0/nTotal);
    printf("Sockets:\t%7ld, %5.2f %%\n\n", nsock, nsock*100.0/nTotal);
    printf("Total:\t%7ld\n", nTotal);

    exit(ret);
}


// Обход дерева каталогов
static int make_tree(const char *filename, int depth, Handler *func)
{
    struct stat statbuf;
    struct dirent * dirp;
    DIR *dp;
    int ret = 0;

    if (lstat(filename, &statbuf) < 0) // ошибка ф-ции stat
        return(func(filename, &statbuf, NSTAT));

    for (int i = 0; i < depth; ++i)
        printf("|\t");

    if (S_ISDIR(statbuf.st_mode) == 0) // не каталог
        return(func(filename, &statbuf, NCAT));

    ret = func(filename, &statbuf, CAT);

    if ((dp = opendir(filename)) == NULL) // каталог недоступен для чтения
        return(func(filename, &statbuf, NREAD));

    chdir(filename);
    while ((dirp = readdir(dp)) != NULL && ret == 0)
    {
        if (strcmp(dirp->d_name, ".") != 0 &&
            strcmp(dirp->d_name, "..") != 0) // пропуск каталогов . и ..
        {
            ret = make_tree(dirp->d_name, depth + 1, func);
        }
    }

    chdir("..");

    if (closedir(dp) < 0)
        perror("Can't close catalog");

    return(ret);
}

static int counter(const char *pathame, const struct stat *statptr, int type)
{
    switch(type)
    {
        case NCAT:
            printf( "-- %s (inode = %lu)\n", pathame, statptr->st_ino);
            switch(statptr->st_mode & S_IFMT)
            {
                case S_IFREG: nreg++; break;
                case S_IFBLK: nblk++; break;
                case S_IFCHR: nchr++; break;
                case S_IFIFO: nfifo++; break;
                case S_IFLNK: nslink++; break;
                case S_IFSOCK: nsock++; break;
                case S_IFDIR:
                    perror("Catalog has type FTW_F");
                    return(-1);
            }
            break;
        case CAT:
            printf( "-- %s (inode = %lu)/\n", pathame, statptr->st_ino);
            ndir++;
            break;
        case NREAD:
            perror("No access to the catalog");
            return(-1);
        case NSTAT:
            perror("Error of stat function");
            return(-1);
        default:
            perror("Unknown file type");
            return(-1);
    }
    return(0);
}
