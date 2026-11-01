/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * Programs the WCN36xx (Pronto) Bluetooth NVM tags over SMD before the stack
 * opens the controller. The CAF libbt-vendor (SMD path) sets
 * vendor.bluetooth.hciattach=true and waits for vendor.bluetooth.status=on;
 * init starts this tool on that trigger. It replaces NvmTagsManager from the
 * stock QTI HAL (android.hardware.bluetooth@1.0-impl-qti.so): same tags,
 * same values, same order, followed by HCI_RESET.
 */

#define LOG_TAG "a02q-bt-nvm"

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include <cutils/properties.h>
#include <log/log.h>

#define SMD_CMD_DEV "/dev/smd3"
#define BDADDR_PATH_PROP "ro.bt.bdaddr_path"
#define READ_TIMEOUT_MS 2000

#define EVT_CMD_COMPLETE 0x0e
#define EVT_VENDOR 0xff
#define NVM_ACCESS_CODE 0x0b /* opcode 0xfc0b, NVM_ACCESS_SET */

#define TAG_BDADDR 2
#define TAG_PCBA 36 /* last byte depends on the WCN chip variant */

struct nvm_tag {
    uint8_t num;
    uint8_t len;
    uint8_t val[12];
};

/* Table copied from the stock HAL (.rodata 0x2b248). */
static struct nvm_tag tags[] = {
    {TAG_BDADDR, 6, {0x77, 0x78, 0x23, 0x01, 0x56, 0x22}},
    {95, 2, {0x00, 0x0b}},
    {83, 3, {0x07, 0x07, 0x07}},
    {TAG_PCBA, 12, {0xff, 0x03, 0x07, 0x09, 0x09, 0x09, 0x00, 0x00, 0x09, 0x09, 0x01, 0x03}},
    {78, 2, {0x08, 0x08}},
};

/* Stock GetPcabInfo(): ROW/LATAM/IN/EUR boards carry HS7X_3615 (0x04), anything else 0x03. */
static uint8_t pcba_chip_byte(void) {
    char pcba[PROPERTY_VALUE_MAX];
    static const char *const regions[] = {"ROW", "LATAM", "IN", "EUR"};

    if (property_get("ro.boot.pcbainfo", pcba, "") <= 0) return 0x03;
    for (size_t i = 0; i < sizeof(regions) / sizeof(regions[0]); i++)
        if (strstr(pcba, regions[i])) return 0x04;
    return 0x03;
}

/* "E4:F3:C4:1E:C2:81" -> 81 c2 1e c4 f3 e4 (the controller wants it little-endian). */
static bool read_bdaddr(uint8_t out[6]) {
    char path[PROPERTY_VALUE_MAX], str[32] = {0};
    unsigned int b[6];
    FILE *f;

    property_get(BDADDR_PATH_PROP, path, "/efs/bluetooth/bt_addr");
    f = fopen(path, "r");
    if (!f) {
        ALOGE("open %s: %s", path, strerror(errno));
        return false;
    }
    bool ok = fgets(str, sizeof(str), f) &&
              sscanf(str, "%2x:%2x:%2x:%2x:%2x:%2x", &b[0], &b[1], &b[2], &b[3], &b[4],
                     &b[5]) == 6;
    fclose(f);
    if (!ok) {
        ALOGE("bad BD address in %s", path);
        return false;
    }
    for (int i = 0; i < 6; i++) out[i] = b[5 - i];
    return true;
}

static int open_smd(void) {
    struct termios term;
    int fd = -1;

    for (int retry = 0; retry < 8 && fd < 0; retry++) {
        if (retry) usleep(500000);
        fd = open(SMD_CMD_DEV, O_RDWR | O_NOCTTY);
    }
    if (fd < 0) {
        ALOGE("open %s: %s", SMD_CMD_DEV, strerror(errno));
        return -1;
    }
    if (tcflush(fd, TCIOFLUSH) < 0 || tcgetattr(fd, &term) < 0) goto err;
    cfmakeraw(&term);
    term.c_cflag |= CRTSCTS | CLOCAL;
    if (tcsetattr(fd, TCSANOW, &term) < 0) goto err;
    return fd;
err:
    ALOGE("configure %s: %s", SMD_CMD_DEV, strerror(errno));
    close(fd);
    return -1;
}

static bool read_full(int fd, uint8_t *buf, size_t len) {
    struct pollfd pfd = {.fd = fd, .events = POLLIN};

    while (len) {
        int r = poll(&pfd, 1, READ_TIMEOUT_MS);
        if (r <= 0) {
            ALOGE("poll: %s", r ? strerror(errno) : "timeout");
            return false;
        }
        ssize_t n = read(fd, buf, len);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) {
            ALOGE("read: %s", n ? strerror(errno) : "EOF");
            return false;
        }
        buf += n;
        len -= n;
    }
    return true;
}

/* SMD carries one HCI channel per device, so packets have no H4 type byte. */
static bool read_event(int fd, uint8_t expect) {
    uint8_t evt[260];

    if (!read_full(fd, evt, 2) || !read_full(fd, evt + 2, evt[1])) return false;
    if (evt[0] != expect || (expect == EVT_VENDOR && (evt[1] < 1 || evt[2] != NVM_ACCESS_CODE))) {
        ALOGE("unexpected event 0x%02x (wanted 0x%02x)", evt[0], expect);
        return false;
    }
    return true;
}

static bool send_cmd(int fd, const uint8_t *cmd, size_t len) {
    if (write(fd, cmd, len) != (ssize_t)len) {
        ALOGE("write: %s", strerror(errno));
        return false;
    }
    return true;
}

static bool program_tags(int fd) {
    for (size_t i = 0; i < sizeof(tags) / sizeof(tags[0]); i++) {
        const struct nvm_tag *t = &tags[i];
        uint8_t cmd[3 + 3 + sizeof(t->val)] = {0x0b, 0xfc, (uint8_t)(3 + t->len), 0x01, t->num,
                                               t->len};

        memcpy(cmd + 6, t->val, t->len);
        if (!send_cmd(fd, cmd, 6 + t->len) || !read_event(fd, EVT_VENDOR) ||
            !read_event(fd, EVT_CMD_COMPLETE)) {
            ALOGE("NVM tag %u failed", t->num);
            return false;
        }
        ALOGI("NVM tag %u set", t->num);
    }

    static const uint8_t hci_reset[] = {0x03, 0x0c, 0x00};
    if (!send_cmd(fd, hci_reset, sizeof(hci_reset)) || !read_event(fd, EVT_CMD_COMPLETE)) {
        ALOGE("HCI_RESET failed");
        return false;
    }
    return true;
}

int main(void) {
    bool ok = false;
    int fd;

    for (size_t i = 0; i < sizeof(tags) / sizeof(tags[0]); i++) {
        if (tags[i].num == TAG_PCBA) tags[i].val[11] = pcba_chip_byte();
        if (tags[i].num == TAG_BDADDR && !read_bdaddr(tags[i].val))
            ALOGW("keeping the default BD address");
    }

    fd = open_smd();
    if (fd >= 0) {
        ok = program_tags(fd);
        close(fd);
    }

    ALOGI("NVM download %s", ok ? "done" : "FAILED");
    property_set("vendor.bluetooth.status", ok ? "on" : "off");
    /* Re-arm the init trigger so the next enable retries after a failure. */
    property_set("vendor.bluetooth.hciattach", "false");
    return ok ? 0 : 1;
}
