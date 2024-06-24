/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GNU_SOURCE
#define GNU_SOURCE
#endif
#include <cstdio>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/qos_interface.h"

static int TrivalOpenQosCtrlNode(void)
{
    char fileName[] = "/proc/thread-self/sched_qos_ctrl";
    int fd = open(fileName, O_RDWR);
#ifdef QOS_DEBUG
    if (fd < 0) {
        printf("task %d belong to user %d open qos node failed\n", getpid(), getuid());
    }
#endif
    return fd;
}

int QosApplyForOther(unsigned int level, int tid)
{
    struct QosCtrlData data;
    int fd;

    int ret;

    fd = TrivalOpenQosCtrlNode();
    if (fd < 0) {
        return fd;
    }

    data.level = level;
    data.type = static_cast<unsigned int>(QosManipulateType::QOS_APPLY);
    data.pid = tid;

    ret = ioctl(fd, QOS_CTRL_BASIC_OPERATION, &data);
#ifdef QOS_DEBUG
    if (ret < 0) {
        printf("qos apply failed for task %d\n", tid);
    }
#endif
    close(fd);
    return ret;
}

int QosLeaveForOther(int tid)
{
    struct QosCtrlData data;
    int fd;
    int ret;

    fd = TrivalOpenQosCtrlNode();
    if (fd < 0) {
        return fd;
    }

    data.type = static_cast<unsigned int>(QosManipulateType::QOS_LEAVE);
    data.pid = tid;

    ret = ioctl(fd, QOS_CTRL_BASIC_OPERATION, &data);
#ifdef QOS_DEBUG
    if (ret < 0) {
        printf("qos leave failed for task %d\n", tid);
    }
#endif
    close(fd);
    return ret;
}

int QosGetForOther(int tid, int &level)
{
    int fd;
    int ret = 0;

    fd = TrivalOpenQosCtrlNode();
    if (fd < 0) {
        return fd;
    }
#ifdef QOS_EXT_ENABLE
    struct QosCtrlData data;
    data.type = static_cast<unsigned int>(QosManipulateType::QOS_GET);
    data.pid = tid;
    data.qos = -1;

    ret = ioctl(fd, QOS_CTRL_BASIC_OPERATION, &data);
#ifdef QOS_DEBUG
    if (ret < 0) {
        printf("get qos failed for task %d\n", tid);
    }
#endif
    level = data.qos;
#endif
    close(fd);
    return ret;
}
