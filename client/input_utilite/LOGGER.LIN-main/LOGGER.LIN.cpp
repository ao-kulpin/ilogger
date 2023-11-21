#include <iostream>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <thread>
#include <linux/input-event-codes.h>
#include <X11/Xlib.h>

void keyboardInput(int fd) {
    struct input_event ev;
    while (true) {
        if (read(fd, &ev, sizeof(ev)) == -1) {
            perror("Failed to read keyboard input");
            break;
        }

        if (ev.type == EV_KEY) {
            std::cout << "Key " << (ev.value ? "press: " : "release: ") << ev.code << std::endl;
        }
    }
}

void mouseInput(int fd) {
        
    Display *display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Error open display X" << std::endl;
        return;
    }

    int screen = DefaultScreen(display);
    int screenWidth = DisplayWidth(display, screen);
    int screenHeight = DisplayHeight(display, screen);

    struct libevdev *dev = nullptr;
    if (libevdev_new_from_fd(fd, &dev) != 0) {
        std::cerr << "Error libevdev" << std::endl;
        close(fd);
        XCloseDisplay(display);
        return;
    }

    int minX = libevdev_get_abs_minimum(dev, ABS_X);
    int minY = libevdev_get_abs_minimum(dev, ABS_Y);
    int maxX = libevdev_get_abs_maximum(dev, ABS_X);
    int maxY = libevdev_get_abs_maximum(dev, ABS_Y);

    // std::cout << screenWidth << " " << screenHeight << std::endl;
    // std::cout << minX << " " << minY << " " << maxX << " " << maxY << std::endl;
    struct input_event ev;

    static int currentX = 0;
    static int currentY = 0;

    while (true) {
        if (read(fd, &ev, sizeof(ev)) == -1) {
            perror("Failed to read mouse input");
            break;
        }

        // std::cout << ev.type << ev.code << ev.value << std::endl;

        if (ev.type == EV_KEY) {
            std::string keyName = "";
            if(ev.code == BTN_LEFT){
                keyName = "LEFT"    ;
            }else if(ev.code == BTN_RIGHT){
                keyName = "RIGHT";
            }else if(ev.code == BTN_MIDDLE){
                keyName = "MIDDLE";
            }
            std::cout << "Mouse button " << (ev.value ? "press: " : "release: ") << keyName << std::endl;
        } else if (ev.type == EV_REL) {
            if(ev.code == REL_WHEEL || ev.code == REL_WHEEL_HI_RES){
                std::cout << "Mouse wheel: " << (ev.value>0 ? "UP" : "DOWN") << std::endl;
            }else if (ev.code == REL_X) {
                int x = ev.value;
                currentX = x;
                std::cout << "Mouse move: X=" << x << ", Y=" << currentY << std::endl;
            } else if (ev.code == REL_Y) {
                int y = ev.value;
                currentY = y;
                std::cout << "Mouse move: X=" << currentX << ", Y=" << y << std::endl;
            }
        }else if (ev.type == EV_ABS){
            if (ev.code == REL_X) {
                int x = ev.value;
                currentX = (x - minX) * screenWidth / (maxX - minX);
                std::cout << "Mouse move: X=" << currentX << ", Y=" << currentY << std::endl;
            } else if (ev.code == REL_Y) {
                int y = ev.value;
                currentY = (y - minY) * screenHeight / (maxY - minY);
                std::cout << "Mouse move: X=" << currentX << ", Y=" << currentY << std::endl;
            }
        }
    }
}

int main(int argc, char** argv) {

    if(argc < 2){
        std::cerr << "Usage: ./LOGGER.LIN  /dev/input/event2 /dev/input/event6" << std::endl;
    }

    int keyboardFd = open(argv[1], O_RDONLY); // Замените X на номер устройства клавиатуры
    int mouseFd = open(argv[2], O_RDONLY);    // Замените Y на номер устройства мыши

    if (keyboardFd == -1 || mouseFd == -1) {
        perror("Failed to open input devices");
        return 1;
    }

    std::thread keyboardThread(keyboardInput, keyboardFd);
    std::thread mouseThread(mouseInput, mouseFd);

    keyboardThread.join();
    mouseThread.join();

    close(keyboardFd);
    close(mouseFd);

    return 0;

}
