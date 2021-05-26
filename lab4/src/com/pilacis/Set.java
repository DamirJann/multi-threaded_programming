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

    }

    Node head = new Node(null);

    @Override
    public boolean add(T value) {

        Node cur = head;

        while (cur.nextNode != null && cur.nextNode.value.compareTo(value) < 0) {
            cur = cur.nextNode;

        }

        if (cur.nextNode != null && cur.nextNode.value.compareTo(value) == 0) {
            return false;
        } else {

            Node newNode = new Node(value);
            newNode.nextNode = cur.nextNode;
            cur.nextNode = newNode;
            return true;
        }


    }

    @Override
    public boolean remove(T value) {
        Node cur = head;
        while (cur.nextNode != null && cur.nextNode.value.compareTo(value) != 0) {
            cur = cur.nextNode;
        }

        if (cur.nextNode != null && cur.nextNode.value.compareTo(value) == 0) {
            cur.nextNode = cur.nextNode.nextNode;
            return true;
        } else {
            return false;
        }
    }


    @Override
    public boolean contains(T value) {
        Node cur = head;
        while (cur.nextNode != null && cur.nextNode.value.compareTo(value) < 0) {
            cur = cur.nextNode;
        }
        if (cur.nextNode != null && cur.nextNode.value.compareTo(value) == 0) {
            return true;
        } else {
            return false;
        }
    }

    @Override
    public boolean isEmpty() {
        return (head.nextNode == null);
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node cur = head.nextNode;
        while (cur != null) {
            str.append(cur.value.toString()).append(" ");
            cur = cur.nextNode;
        }
        return str.toString();
    }
}