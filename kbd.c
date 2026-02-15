// kbd-rgb.c — tiny, safe-ish setuid helper for TUXEDO keyboard RGB
// Writes only to a fixed sysfs path: /sys/class/leds/rgb:kbd_backlight/multi_intensity
//
// Usage:
//   kbd-rgb R G B        (0..255 each)
//   kbd-rgb LEVEL        (0..255)
//   kbd-rgb --get
//
// Security measures:
// - Only members of group "kbdlight" may use it
// - Validates inputs strictly
// - Writes only to a hardcoded sysfs file

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef SYSFS_PATH
#define SYSFS_PATH "/sys/class/leds/rgb::kbd_backlight"
#endif

#define SYSFS_PATH_COLOR      SYSFS_PATH "/multi_intensity"
#define SYSFS_PATH_BRIGHTNESS  SYSFS_PATH "/brightness"

#ifndef ALLOW_GROUP
#define ALLOW_GROUP "kbdlight"
#endif /* ifndef ALLOW_GROUP */

static int is_user_in_group(gid_t uid, const char *group_name) {
    struct group *gr = getgrnam(group_name);
    if (!gr) return 0;

    // Primary group check
    // We need user's groups; easiest safe-ish check: getgrouplist.
    // But we can use getgroups() for current process, which (for setuid) reflects real user groups after init.
    // We will switch to real user's supplementary groups automatically (normally inherited).
    int ngroups = getgroups(0, NULL);
    if (ngroups < 0) return 0;

    gid_t *groups = calloc((size_t)ngroups, sizeof(gid_t));
    if (!groups) return 0;

    if (getgroups(ngroups, groups) < 0) {
        free(groups);
        return 0;
    }

    int ok = 0;
    for (int i = 0; i < ngroups; i++) {
        if (groups[i] == gr->gr_gid) { ok = 1; break; }
    }
    free(groups);
    return ok;
}

static int parse_u8(const char *s, int *out) {
    if (!s || !*s) return 0;
    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0') return 0;
    if (v < 0 || v > 255) return 0;
    *out = (int)v;
    return 1;
}

static int do_get(void) {
    int fd = open(SYSFS_PATH_COLOR, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", SYSFS_PATH_COLOR, strerror(errno));
        return 1;
    }
    char buf[128];
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    close(fd);
    if (n < 0) {
        fprintf(stderr, "read(%s) failed: %s\n", SYSFS_PATH_COLOR, strerror(errno));
        return 1;
    }
    buf[n] = '\0';
    // Print as-is (usually: "R G B\n" or similar)
    fputs(buf, stdout);
    return 0;
}

static int do_set_color(int r, int g, int b) {
    // sysfs usually expects: "R G B\n"
    char out[64];
    int len = snprintf(out, sizeof(out), "%d %d %d\n", r, g, b);
    if (len <= 0 || len >= (int)sizeof(out)) {
        fprintf(stderr, "snprintf failed\n");
        return 1;
    }

    int fd = open(SYSFS_PATH_COLOR, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", SYSFS_PATH_COLOR, strerror(errno));
        return 1;
    }
    ssize_t n = write(fd, out, (size_t)len);
    close(fd);
    if (n != len) {
        fprintf(stderr, "write(%s) failed: %s\n", SYSFS_PATH_COLOR, (n < 0) ? strerror(errno) : "short write");
        return 1;
    }
    return 0;
}

static int do_set_brightness(int level) {
    // sysfs usually expects: "R G B\n"
    char out[64];
    int len = snprintf(out, sizeof(out), "%d\n", level);
    if (len <= 0 || len >= (int)sizeof(out)) {
        fprintf(stderr, "snprintf failed\n");
        return 1;
    }

    int fd = open(SYSFS_PATH_BRIGHTNESS, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "open(%s) failed: %s\n", SYSFS_PATH_BRIGHTNESS, strerror(errno));
        return 1;
    }
    ssize_t n = write(fd, out, (size_t)len);
    close(fd);
    if (n != len) {
        fprintf(stderr, "write(%s) failed: %s\n", SYSFS_PATH_BRIGHTNESS, (n < 0) ? strerror(errno) : "short write");
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    // Authorization: only allow real user who is in group ALLOW_GROUP
    uid_t ruid = getuid();   // real uid
    uid_t euid = geteuid();  // effective uid (will be 0 when setuid root)

    (void)euid;

    if (!is_user_in_group(ruid, ALLOW_GROUP)) {
        fprintf(stderr, "Not allowed. Add your user to group '%s'.\n", ALLOW_GROUP);
        return 2;
    }

    // Safety: refuse to run if sysfs path doesn't exist
    struct stat st;
    if (stat(SYSFS_PATH_COLOR, &st) != 0 || stat(SYSFS_PATH_BRIGHTNESS, &st) != 0) {
        fprintf(stderr, "Missing sysfs node: %s (%s)\n", SYSFS_PATH_COLOR, strerror(errno));
        return 3;
    }

    if (argc == 2) {
        if (strcmp(argv[1], "--get") == 0)
            return do_get();
        int level;
        if (!parse_u8(argv[1], &level)) {
            fprintf(stderr, "Invalid value. Use integer 0..255.\n");
            return 1;
        }
        return do_set_brightness(level);
    }


    if (argc != 4) {
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "  %s R G B   (0..255)\n", argv[0]);
        fprintf(stderr, "  %s LEVEL   (0..255)\n", argv[0]);
        fprintf(stderr, "  %s --get\n", argv[0]);
        return 1;
    }

    int r, g, b;
    if (!parse_u8(argv[1], &r) || !parse_u8(argv[2], &g) || !parse_u8(argv[3], &b)) {
        fprintf(stderr, "Invalid values. Use integers 0..255.\n");
        return 1;
    }

    return do_set_color(r, g, b);
}
