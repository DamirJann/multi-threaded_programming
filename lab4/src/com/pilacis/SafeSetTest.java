package com.pilacis;

import org.junit.Assert;
import org.junit.Test;

class TestThread1 extends Thread {
    private SafeSet<Integer> set;

    public TestThread1(SafeSet<Integer> set) {
        this.set = set;
    }

    public void run() {
        System.out.println(set);
        for (int i = 10; i < 500000000; i++) {
            try {
                sleep(1);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            set.add(i);
        }
        for (int i = 20; i < 500000000; i++) {
            set.remove(i);
        }
    }
}

class TestThread2 extends Thread {
    private final SafeSet set;

    public TestThread2(SafeSet set) {
        this.set = set;
    }

    public void run() {
        for (int i = 0; i < 50000000; i++) {
            set.add(i);
        }
    }
}


public class SafeSetTest {
    @Test
    public void test1() throws InterruptedException {
        String answer = "4 ";
        SafeSet<Integer> set = new SafeSet<>();
        set.add(4);
        set.add(4);
        set.add(4);
        set.add(4);
        Assert.assertEquals(answer, set.getElements());
    }

    @Test
    public void test2() throws InterruptedException {
        String answer = "";
        SafeSet<Integer> set = new SafeSet<>();
        set.add(4);
        set.add(4);
        set.add(4);
        set.add(4);
        set.remove(4);
        Assert.assertEquals(answer, set.getElements());
    }

    @Test
    public void test3() throws InterruptedException {
        String answer = "-4 2 5 10 2";
        SafeSet<Integer> set = new SafeSet<>();
        set.add(5);
        set.add(2);
        set.add(10);
        set.add(-4);
        set.remove(4);
        Assert.assertEquals(answer, set.getElements());
    }
}