package com.pilacis;

import java.util.ArrayList;

/**
 * Lock-Free множество.
 *
 * @param <T> тип ключей
 */
public interface Set<T extends Comparable<T>> {
    /**
     * Добавить ключ к множеству
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если value уже существует в множестве,
     * true если элемент был добавлен
     */
    boolean add(T value);

    /**
     * Удалить ключ из множества
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @param value значение ключа
     * @return false если ключ не был найден, true если ключ успешно удален
     */
    boolean remove(T value);

    /**
     * Проверка наличия ключа в множестве
     * <p>
     * Алгоритм должен быть как минимум wait-free
     *
     * @param value значение ключа
     * @return true если элемент содержится в множестве, иначе - false
     */
    boolean contains(T value);

    /**
     * Проверка множества на пустоту
     * <p>
     * Алгоритм должен быть как минимум lock-free
     *
     * @return true если множество пусто, иначе - false
     */
    boolean isEmpty();
}


class SafeSet<T extends Comparable<T>> implements Set<T> {

    static class Node<T extends Comparable<T>> {
        Node(T value) {
            this.value = value;
            nextNode = null;
        }

        T value;
        Node nextNode;

        boolean isStart(){
            return value == null;
        }
    }

    Node head = new Node(null);

    @Override
    public boolean add(T value) {
        Node cur = head;
        while (cur.isStart() && cur.nextNode != null && cur.value.compareTo(value) < 0) {
            cur = head.nextNode;
        }


        if (!cur.isStart() && cur.value.compareTo(value) == 0){
            Node nextNode = cur.nextNode;
            cur.nextNode = new Node(value);
            cur.nextNode.nextNode = nextNode;
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean remove(T value) {
        Node cur = head;
        while (cur != null && cur.nextNode != null && cur.nextNode.value.compareTo(value) < 0) {
            cur = head.nextNode;
        }
        if (cur != null && cur.nextNode != null && cur.nextNode.value.compareTo(value) == 0) {
            cur.nextNode = cur.nextNode.nextNode;
            return true;
        } else if (cur != null && cur.value.compareTo(value) == 0) {
            head = null;
            return true;
        } else {
            return false;
        }
    }



    @Override
    public boolean contains(T value) {
        Node cur = head;
        while (cur != null && cur.nextNode != null && cur.value.compareTo(value) < 0) {
            cur = head.nextNode;
        }
        if (cur != null && cur.nextNode.value.compareTo(value) == 0) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean isEmpty() {
        return (head != null && head.value == null);
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node cur = head;
        while (cur != null) {
            str.append(cur.value.toString()).append(" ");
            cur = cur.nextNode;
        }
        return str.toString();
    }
}