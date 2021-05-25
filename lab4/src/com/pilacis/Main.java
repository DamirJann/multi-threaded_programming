package com.pilacis;

public class Main {

    public static void main(String[] args) {
        SafeSet<Integer> set = new SafeSet<>();
        set.add(4);
        set.add(4);
        set.add(4);
        set.add(4);
        set.remove(4);
        System.out.println(set.getElements());
    }

}
