#ifndef CAMERA_H
#define CAMERA_H

/*

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 */
#include <CImg.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#include <stdint.h>
#include <linux/videodev2.h>
#include <libv4l2.h>
#include <map>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>

using namespace std;
using namespace cimg_library;

typedef uint8_t UINT8;

//Much of this code has been stolen from http://linuxtv.org/downloads/v4l-dvb-apis/v4l2grab-example.html
#define CLEAR(x) memset(&(x), 0, sizeof(x))

static void xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = v4l2_ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
        fprintf(stderr, "error %d, %s\n", errno, strerror(errno));
        abort();
    }
}

class Camera {
public:
    Camera(const char *dev_name, int width, int height, int exposure, int whitepoint) {
        struct v4l2_buffer              buf;
        struct v4l2_requestbuffers      req;
        enum v4l2_buf_type              type;
        int                             r;
        unsigned int                    i;

        fd = v4l2_open(dev_name, O_RDWR | O_NONBLOCK, 0);
        if (fd < 0) {
            perror("Cannot open device");
            exit(EXIT_FAILURE);
        }

        CLEAR(fmt);
        fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        fmt.fmt.pix.width       = 640;
        fmt.fmt.pix.height      = 480;
        fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
        fmt.fmt.pix.field       = V4L2_FIELD_NONE;
        xioctl(fd, VIDIOC_S_FMT, &fmt);
        if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) {
            printf("Libv4l didn't accept RGB24 format. Can't proceed.\n");
            exit(EXIT_FAILURE);
        }
        if ((fmt.fmt.pix.width != 640) || (fmt.fmt.pix.height != 480))
            printf("Warning: driver is sending image at %dx%d\n",
                   fmt.fmt.pix.width, fmt.fmt.pix.height);
        set_attr(V4L2_CID_EXPOSURE_AUTO, V4L2_EXPOSURE_SHUTTER_PRIORITY);
        set_attr(V4L2_CID_EXPOSURE_ABSOLUTE, exposure);
        set_attr(V4L2_CID_AUTO_WHITE_BALANCE, 0);
        set_attr(V4L2_CID_WHITE_BALANCE_TEMPERATURE, whitepoint);
        CLEAR(req);
        req.count = 1;
        req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        req.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_REQBUFS, &req);

        buffers = static_cast<struct buffer*>(calloc(req.count, sizeof(*buffers)));
        for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
            CLEAR(buf);

            buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory      = V4L2_MEMORY_MMAP;
            buf.index       = n_buffers;

            xioctl(fd, VIDIOC_QUERYBUF, &buf);

            buffers[n_buffers].length = buf.length;
            buffers[n_buffers].start = v4l2_mmap(NULL, buf.length,
                                                 PROT_READ | PROT_WRITE, MAP_SHARED,
                                                 fd, buf.m.offset);

            if (MAP_FAILED == buffers[n_buffers].start) {
                perror("mmap");
                exit(EXIT_FAILURE);
            }
        }

        for (i = 0; i < n_buffers; ++i) {
            CLEAR(buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            xioctl(fd, VIDIOC_QBUF, &buf);
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        xioctl(fd, VIDIOC_STREAMON, &type);
    }
    ~Camera() {
        int i;
        enum v4l2_buf_type              type;
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        xioctl(fd, VIDIOC_STREAMOFF, &type);
        for (i = 0; i < n_buffers; ++i)
            v4l2_munmap(buffers[i].start, buffers[i].length);
        v4l2_close(fd);
    }
    void load(uint8_t **out, int *width, int *height) {
        int r;
        fd_set                          fds;
        struct timeval                  tv;
        struct v4l2_buffer              buf;
        do {
            FD_ZERO(&fds);
            FD_SET(fd, &fds);

            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;

            r = select(fd + 1, &fds, NULL, NULL, &tv);
        } while ((r == -1 && (errno = EINTR)));
        if (r == -1) {
            perror("select");
            exit(errno);
        }

        CLEAR(buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        xioctl(fd, VIDIOC_DQBUF, &buf);

        /*sprintf(out_name, "out%03d.ppm", i);
        fout = fopen(out_name, "w");
        if (!fout) {
                perror("Cannot open image");
                exit(EXIT_FAILURE);
        }
        fprintf(fout, "P6\n%d %d 255\n",
                fmt.fmt.pix.width, fmt.fmt.pix.height);
        fwrite(buffers[buf.index].start, buf.bytesused, 1, fout);
        fclose(fout);*/
        *width=fmt.fmt.pix.width;
        *height=fmt.fmt.pix.height;
        *out=static_cast<uint8_t*>(buffers[buf.index].start);
        /*img.assign(fmt.fmt.pix.width, fmt.fmt.pix.height, 1, 3);
        UINT8
            *ptr_r = img.data(0,0,0,0),
            *ptr_g = img.data(0,0,0,1),
            *ptr_b = img.data(0,0,0,2);
          UINT8 *ptrs=static_cast<uint8_t*>(buffers[buf.index].start);
          while(ptrs<(static_cast<uint8_t*>(buffers[buf.index].start)+buf.bytesused)){
              *(ptr_r++) = (UINT8)*(ptrs++);
              *(ptr_g++) = (UINT8)*(ptrs++);
              *(ptr_b++) = (UINT8)*(ptrs++);
          }*/

        xioctl(fd, VIDIOC_QBUF, &buf);
    }
private:
    struct buffer {
        void   *start;
        size_t length;
    };

    int fd;
    unsigned int n_buffers;
    struct buffer                   *buffers;
    struct v4l2_format              fmt;
    void set_attr(int id, int value){
        struct v4l2_queryctrl queryctrl={0};
        queryctrl.id=id;
        xioctl(fd, VIDIOC_QUERYCTRL, &queryctrl);
        if(queryctrl.flags&V4L2_CTRL_FLAG_DISABLED){
            fprintf(stderr, "ID: %d not supported\n", id);
            abort();
        }
        struct v4l2_control control={0};
        control.id=id;
        control.value=value;
        xioctl(fd, VIDIOC_S_CTRL, &control);
    }
};

#endif
