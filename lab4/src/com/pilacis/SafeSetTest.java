package com.pilacis;

import com.pilacis.SafeSet;
import org.junit.Assert;
import org.junit.Test;

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
        String answer = "-4 2 5 10 ";
        SafeSet<Integer> set = new SafeSet<>();
        set.add(5);
        set.add(2);
        set.add(10);
        set.add(-4);
        set.remove(4);
        Assert.assertEquals(answer, set.getElements());
    }

    @Test
    public void test4() throws InterruptedException {
        String answer = "-43 5 55 ";
        SafeSet<Integer> set = new SafeSet<>();
        set.add(5);
        set.add(2);
        set.add(10);
        set.add(-4);
        set.remove(5);
        set.remove(2);
        set.remove(10);
        set.remove(-4);
        Assert.assertTrue(set.isEmpty());
        set.add(5);
        set.add(-43);
        set.add(55);
        Assert.assertEquals(answer, set.getElements());
    }


    @Test
    public void test5() throws InterruptedException {
        SafeSet<Integer> set = new SafeSet<>();
        set.add(5);
        set.add(2);
        set.add(10);
        set.add(-4);

        Assert.assertTrue(set.contains(5));
        Assert.assertFalse(set.contains(35));
    }
}