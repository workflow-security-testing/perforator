package tech.perforator.unwind;

class Native {
    static {
        System.loadLibrary("unwind-jni");
    }

    private Native() {
    }

    static native void unwind0(long impl);
    static native void unwindIfZero0(long impl, int x);
    static native long make0();
}
