#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

//This function is for Step 4
char * time2str(const time_t * when, long ns) {
  char * ans = malloc(128 * sizeof(*ans));
  char temp1[64];
  char temp2[32];
  const struct tm * t = localtime(when);
  strftime(temp1, 512, "%Y-%m-%d %H:%M:%S", t);
  strftime(temp2, 32, "%z", t);
  snprintf(ans, 128, "%s.%09ld %s", temp1, ns, temp2);
  return ans;
}

//function to determine file type
char * printType(struct stat sb) {
  char * type = NULL;
  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:
      type = "block special file";
      break;
    case S_IFCHR:
      type = "character special file";
      break;
    case S_IFDIR:
      type = "directory";
      break;
    case S_IFIFO:
      type = "fifo";
      break;
    case S_IFLNK:
      type = "symbolic link";
      break;
    case S_IFREG:
      type = "regular file";
      break;
    case S_IFSOCK:
      type = "socket";
      break;
    default:
      type = "unknown?";
      break;
  }
  return type;
}

//function to determine permission[0]
char get_permission0(char permission[], struct stat sb) {
  switch (sb.st_mode & S_IFMT) {
    case S_IFBLK:
      permission[0] = 'b';
      break;
    case S_IFCHR:
      permission[0] = 'c';
      break;
    case S_IFDIR:
      permission[0] = 'd';
      break;
    case S_IFIFO:
      permission[0] = 'p';
      break;
    case S_IFLNK:
      permission[0] = 'l';
      break;
    case S_IFREG:
      permission[0] = '-';
      break;
    case S_IFSOCK:
      permission[0] = 's';
      break;
    default:
      perror("File type is unknown\n");
      exit(EXIT_FAILURE);
  }
  return permission[0];
}
//get permission[]
void getPermissions(char permission[], struct stat sb, int IR, int IW, int IX) {
  if (sb.st_mode & IR) {
    permission[0] = 'r';
  }
  else {
    permission[0] = '-';
  }

  if (sb.st_mode & IW) {
    permission[1] = 'w';
  }
  else {
    permission[1] = '-';
  }

  if (sb.st_mode & IX) {
    permission[2] = 'x';
  }
  else {
    permission[2] = '-';
  }
}

int main(int argc, char * argv[]) {
  struct stat sd;
  if (argc < 2) {
    fprintf(stderr, "error\n");
    exit(EXIT_FAILURE);
  }
  if (lstat(argv[1], &sd) == -1) {
    perror("lstat");
    exit(EXIT_FAILURE);
  }

  for (int i = 1; i < argc; i++) {  //step 7
    if (S_ISLNK(sd.st_mode)) {
      char linktarget[256];
      ssize_t len = readlink(argv[i], linktarget, 256);
      if (len < 0) {
        perror("error\n");
        exit(EXIT_FAILURE);
      }
      if (len > 255) {
        linktarget[255] = '\0';
      }
      else {
        linktarget[len] = '\0';
      }
      printf("  File: %s -> %s\n", argv[i], linktarget);
    }

    else {
      printf("  File: %s\n", argv[i]);  //print the filename
    }                                   //print the size, #blocks, IO BLOCK and the type of the file
    printf("  Size: %-10lu\tBlocks: %-10lu IO Block: %-6lu %s\n",
           sd.st_size,
           sd.st_blocks,
           sd.st_blksize,
           printType(sd));  //print the third line
    if (S_ISCHR(sd.st_mode) || S_ISBLK(sd.st_mode)) {
      printf("Device: %lxh/%lud\tInode: %-10lu  Links: %-5lu Device type: %d,%d\n",
             sd.st_dev,
             sd.st_dev,
             sd.st_ino,
             sd.st_nlink,
             major(sd.st_rdev),
             minor(sd.st_rdev));
    }
    else {
      printf("Device: %lxh/%lud\tInode: %-10lu  Links: %lu\n",
             sd.st_dev,
             sd.st_dev,
             sd.st_ino,
             sd.st_nlink);
    }  //print part of the fourth line
    char p[11];
    get_permission0(p, sd);
    getPermissions(p + 1, sd, S_IRUSR, S_IWUSR, S_IXUSR);
    getPermissions(p + 4, sd, S_IRGRP, S_IWGRP, S_IXGRP);
    getPermissions(p + 7, sd, S_IROTH, S_IWOTH, S_IXOTH);
    p[10] = '\0';
    printf("Access: (%04o/%s)  Uid: (%5d/%8s)   Gid: (%5d/%8s)\n",
           sd.st_mode & ~S_IFMT,
           p,
           sd.st_uid,
           getpwuid(sd.st_uid)->pw_name,
           sd.st_gid,
           getgrgid(sd.st_gid)->gr_name);  //print the last four lines of output
    char * Access = time2str(&sd.st_atime, sd.st_atim.tv_nsec);
    char * Modify = time2str(&sd.st_mtime, sd.st_mtim.tv_nsec);
    char * Change = time2str(&sd.st_ctime, sd.st_ctim.tv_nsec);
    printf("Access: %s\n", Access);
    printf("Modify: %s\n", Modify);
    printf("Change: %s\n", Change);
    printf(" Birth: -\n");
    free(Access);
    free(Modify);
    free(Change);
  }

  return EXIT_SUCCESS;
}
