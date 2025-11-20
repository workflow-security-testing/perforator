package tech.perforator.unwind;

public class Unwinder {
    private final long impl;

    public Unwinder() {
        impl = Native.make0();
    }

    public void unwind() {
        Native.unwind0(impl);
    }

    public void unwindIfZero(int x) {
        Native.unwindIfZero0(impl, x);
    }
}

