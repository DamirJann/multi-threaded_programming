package com.pilacis;

import java.util.ArrayList;
import java.util.concurrent.locks.ReentrantLock;

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
            lock = new ReentrantLock();
            nextNode = null;
        }

        T value;
        ReentrantLock lock;
        Node nextNode;

        boolean isLast() {
            return this.nextNode.nextNode == null;
        }
    }

    SafeSet() {
        head = new Node(null);
        head.nextNode = new Node(null);
    }

    Node head;

    public Node<T> find(T value) {
        Node cur = head;
        Node previousNode = head;
        previousNode.lock.lock();
        do {

            cur.nextNode.lock.lock();
            if (cur.value != null) {
                previousNode.lock.unlock();
            }
            previousNode = cur;

            if (cur.nextNode.nextNode != null && cur.nextNode.value.compareTo(value) < 0) {
                cur = cur.nextNode;
            } else {
                break;
            }
        } while (true);
        return cur;
    }

    @Override
    public boolean add(T value) {

        Node cur = find(value);

        if (!cur.isLast() && cur.nextNode.value.compareTo(value) == 0) {
            cur.nextNode.lock.unlock();
            cur.lock.unlock();
            return false;
        } else {
            Node newNode = new Node(value);
            newNode.nextNode = cur.nextNode;
            cur.nextNode = newNode;
            newNode.nextNode.lock.unlock();
            cur.lock.unlock();
            return true;
        }
    }

    @Override
    public boolean remove(T value) {
        Node cur = find(value);

        if (!cur.isLast() && cur.nextNode.value.compareTo(value) == 0) {
            cur.nextNode.lock.unlock();
            cur.nextNode = cur.nextNode.nextNode;
            cur.lock.unlock();
            return true;
        } else {
            cur.lock.unlock();
            cur.nextNode.lock.unlock();
            return false;
        }
    }


    @Override
    public boolean contains(T value) {
        Node cur = find(value);

        boolean result = (!cur.isLast() && cur.nextNode.value.compareTo(value) == 0);

        cur.nextNode.lock.unlock();
        cur.lock.unlock();

        return result;
    }

    @Override
    public boolean isEmpty() {
        return head.nextNode.value == null;
    }

    public String getElements() {
        StringBuilder str = new StringBuilder();
        Node cur = head.nextNode;
        while (cur.value != null) {
            str.append(cur.value.toString()).append(" ");
            cur = cur.nextNode;
        }
        return str.toString();
    }
}