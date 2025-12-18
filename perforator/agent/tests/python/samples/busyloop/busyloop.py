import os


def bar():
    x = 0
    for _ in range(1):
        x += 1
    return x


def a():
    x = 0
    for _ in range(1):
        x += 1
    return x


def b():
    x = 0
    for _ in range(1):
        x += 1
    return x


def foo():
    y = 1
    while True:
        y += bar()
        y += a()
        y += b()
        if y > 1e9:
            y = 0


def simple():
    foo()


def main():
    print("Current process PID: %d" % os.getpid())
    simple()


if __name__ == "__main__":
    main()
